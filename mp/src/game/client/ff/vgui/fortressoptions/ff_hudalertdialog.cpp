#include "cbase.h"
#include "ff_hudalertdialog.h"

#include "ff_colorpicker.h"
#include <vgui_controls/Button.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/Label.h>

using namespace vgui;

CFFHudAlertDialog::CFFHudAlertDialog(Panel* parent, KeyValues* kvValues, void* pHudPanel)
	: BaseClass(parent, "HudAlertDialog"), m_pHudPanel(pHudPanel)
{
	SetScheme(parent->GetScheme());
	SetProportional(false);
	SetTitle("#HudPanel_Option_DamageAlert", true);
	SetSizeable(false);
	SetDeleteSelfOnClose(true);
	AddActionSignalTarget(parent);
	m_pTarget = new ComboBox(
		this,
		"AlertTarget",
		3,
		false);
	AddChoice(m_pTarget, "#HudPanel_Alert_None", 0);
	AddChoice(m_pTarget, "#HudPanel_Alert_Header", 1);
	AddChoice(m_pTarget, "#HudPanel_Alert_Panel", 2);
	SelectValue(m_pTarget, kvValues->GetInt("AlertTarget"));
	m_pDuration = new ComboBox(
		this,
		"AlertDuration",
		3,
		false);
	AddChoice(m_pDuration, "#HudPanel_Alert_DurationShort", 300);
	AddChoice(m_pDuration, "#HudPanel_Alert_DurationMedium", 800);
	AddChoice(m_pDuration, "#HudPanel_Alert_DurationLong", 1500);
	SelectValue(m_pDuration, kvValues->GetInt("AlertDuration", 800));
	m_pColor = new FFColorPicker(this, "AlertColor");
	m_pColor->SetColorModeSelectionEnabled(false);
	m_pColor->SetValue(
		kvValues->GetInt("AlertColorRed", 255),
		kvValues->GetInt("AlertColorGreen", 80),
		kvValues->GetInt("AlertColorBlue", 80),
		kvValues->GetInt("AlertColorAlpha", 255));

	new Label(this, "TargetLabel", "#HudPanel_Option_AlertTarget");
	new Label(this, "DurationLabel", "#HudPanel_Option_Duration");
	const char* components[] = { "Red", "Green", "Blue", "Alpha" };
	const char* captions[] = { "#GameUI_Red", "#GameUI_Green", "#GameUI_Blue", "#GameUI_Alpha" };
	for (int i = 0; i < 4; ++i)
	{
		char name[64];
		Q_snprintf(
			name,
			sizeof(name),
			"%sLabel",
			components[i]);
		new Label(this, name, captions[i]);
	}

	new Button(
		this,
		"OK",
		"#GameUI_OK",
		this,
		"OK");
	new Button(
		this,
		"Cancel",
		"#GameUI_Cancel",
		this,
		"Cancel");
	LoadControlSettings("resource/ui/FFHudAlertDialog.res");
	MoveToCenterOfScreen();
}

void CFFHudAlertDialog::OnCommand(const char* command)
{
	if (Q_stricmp(command, "OK") == 0)
	{
		KeyValues* kvMessage = new KeyValues("AlertSettingsChanged");
		kvMessage->SetPtr("hudPanel", m_pHudPanel);
		KeyValues* kvValues = new KeyValues("Values");
		kvValues->SetInt("AlertTarget", m_pTarget->GetActiveItemUserData()->GetInt("value"));
		kvValues->SetInt("AlertDuration", m_pDuration->GetActiveItemUserData()->GetInt("value"));
		int red, green, blue, alpha;
		m_pColor->GetValue(
			red,
			green,
			blue,
			alpha);
		kvValues->SetInt("AlertColorRed", red);
		kvValues->SetInt("AlertColorGreen", green);
		kvValues->SetInt("AlertColorBlue", blue);
		kvValues->SetInt("AlertColorAlpha", alpha);
		kvMessage->AddSubKey(kvValues);
		PostActionSignal(kvMessage);
		Close();
	}
	else if (Q_stricmp(command, "Cancel") == 0)
		Close();
	else
		BaseClass::OnCommand(command);
}

void CFFHudAlertDialog::AddChoice(ComboBox* combo, const char* caption, int value)
{
	KeyValues* kvChoice = new KeyValues("Choice");
	kvChoice->SetInt("value", value);
	combo->AddItem(caption, kvChoice);
	kvChoice->deleteThis();
}

void CFFHudAlertDialog::SelectValue(ComboBox* combo, int value)
{
	combo->ActivateItemByRow(0);
	for (int i = 0; i < combo->GetItemCount(); ++i)
	{
		int item = combo->GetItemIDFromRow(i);
		if (combo->GetItemUserData(item)->GetInt("value") == value)
			combo->ActivateItem(item);
	}
}
