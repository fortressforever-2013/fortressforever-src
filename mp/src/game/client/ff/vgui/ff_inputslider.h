/********************************************************************
	created:	2006/09/22
	created:	22:9:2006   20:04
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff\vgui\ff_button.h
	file path:	f:\ff-svn\code\trunk\cl_dll\ff\vgui
	file base:	ff_inputslider
	file ext:	h
	author:		Adam "Elmo" Willden

	purpose:	This just keeps a slider hooked up with a text element.

	notes:

	Used to reside in ff_options but I'm separting the classes into their own files
	Used to be named CInputSlider - why have FFButton and not CFFInputSlider...
	or CButton and CInputSlider. Unless I'm missing the point here.

*********************************************************************/

#ifndef FF_INPUTSLIDER_H
#define FF_INPUTSLIDER_H

#include "vgui_controls/Slider.h"
#include "vgui_controls/TextEntry.h"
#include "KeyValues.h"	// for PostActionSignal("SliderMoved")

namespace vgui
{
	class CFFInputSlider : public Slider
	{
		DECLARE_CLASS_SIMPLE(CFFInputSlider, Slider)

		// Small helper that forwards arrow keys to the owning slider when the box has focus
		class CLinkedTextEntry : public TextEntry
		{
			DECLARE_CLASS_SIMPLE(CLinkedTextEntry, TextEntry)
		public:
			CLinkedTextEntry(Panel* parent, const char* name, CFFInputSlider* pOwner)
				: TextEntry(parent, name), m_pOwner(pOwner) {
			}

			void OnKeyCodeTyped(KeyCode code) override
			{
				if (!m_pOwner)
				{
					TextEntry::OnKeyCodeTyped(code);
					return;
				}

				int delta = 0;
				switch (code)
				{
				case KEY_UP:
				case KEY_RIGHT:
					delta = +1;
					break;

				case KEY_DOWN:
				case KEY_LEFT:
					delta = -1;
					break;

				default:
					// Let normal editing keys behave as usual
					TextEntry::OnKeyCodeTyped(code);
					return;
				}

				// Use the TEXT range (optionally wider than slider), not the slider range.
				int currentValue = m_pOwner->GetInputValue();
				m_pOwner->SetTextAndSync(currentValue + delta, true);
			}

		private:
			CFFInputSlider* m_pOwner;
		};

	public:

		//-----------------------------------------------------------------------------
		// Purpose: Link this slider in with an input box
		//-----------------------------------------------------------------------------
		CFFInputSlider(Panel* parent, char const* panelName, char const* inputName)
			: BaseClass(parent, panelName)
		{
			m_pInputBox = new CLinkedTextEntry(parent, inputName, this);
			m_pInputBox->SetAllowNumericInputOnly(true);
			m_pInputBox->AddActionSignalTarget(this);

			AddActionSignalTarget(parent);
		}

		//-----------------------------------------------------------------------------
		// Purpose: Transfer the value onto the input box
		//-----------------------------------------------------------------------------
		void SetValue(int value, bool bTriggerChangeMessage = true) override
		{
			if (value < _range[0] || value > _range[1])
				return;

			m_pInputBox->SetText(VarArgs("%d", value));
			BaseClass::SetValue(value, bTriggerChangeMessage);
		}

		//-----------------------------------------------------------------------------
		// Purpose: Prefer the value from the input box
		//-----------------------------------------------------------------------------
		int GetValue() override
		{
			return GetInputValue();
		}


		//-----------------------------------------------------------------------------
		// Purpose: When the slider moves, reposition the input box
		//-----------------------------------------------------------------------------
		void SetPos(int x, int y)
		{
			int iWide, iTall;
			GetSize(iWide, iTall);
			m_pInputBox->SetPos(x + iWide, y);
			BaseClass::SetPos(x, y);
		}

		//-----------------------------------------------------------------------------
		// Purpose: Keep the input box up to date
		//-----------------------------------------------------------------------------
		void SetEnabled(bool state) override
		{
			m_pInputBox->SetEnabled(state);

			if (state)
			{
				// Make the TextEntry control editable again (disable removes it)
				m_pInputBox->SetEditable(true);
			}

			BaseClass::SetEnabled(state);
		}

		//-----------------------------------------------------------------------------
		// Purpose: Keep the input box up to date
		//-----------------------------------------------------------------------------
		void SetVisible(bool state) override
		{
			m_pInputBox->SetVisible(state);
			BaseClass::SetVisible(state);
		}

		//-----------------------------------------------------------------------------
		// Purpose: Overloads to optionally set a wider/narrower text range alongside slider range
		//-----------------------------------------------------------------------------
		void SetRange(int min, int max) override
		{
			BaseClass::SetRange(min, max);
			// Unset custom text bounds; text will follow slider range (but still ±9999 capped)
			m_bUseInputBounds = false;
			// Ensure text shows something valid if current text is out-of-range
			int tv = ClampToBounds(GetInputValue());
			SetTextAndSync(tv, true);
		}

		void SetRange(int min, int max, int textMin, int textMax)
		{
			BaseClass::SetRange(min, max);

			// Ensure text input bounds are not smaller than the given slider range
			m_iInputMin = min(min, textMin);
			m_iInputMax = max(max, textMax);
			m_bUseInputBounds = true;

			// Normalise current text to new bounds and sync
			int tv = ClampToBounds(GetInputValue());
			SetTextAndSync(tv, true);
		}

		//-----------------------------------------------------------------------------
		// Purpose: Helper to clamp to text-bounds (±9999 and optional custom bounds)
		//			When m_bUseInputBounds == false, also clamp to the slider's range.
		//-----------------------------------------------------------------------------
		int ClampToBounds(int v)
		{
			// hard cap absolute values beyond 9999 first
			v = clamp(v, -9999, 9999);

			if (m_bUseInputBounds)
			{
				// then optional user-defined bounds
				v = clamp(v, m_iInputMin, m_iInputMax);
			}
			else
			{
				// no custom text bounds => respect slider's own range
				int sMin, sMax; GetRange(sMin, sMax);
				v = clamp(v, sMin, sMax);
			}

			return v;
		}

		//-----------------------------------------------------------------------------
		// Purpose: Set text (respecting text bounds) and sync slider (slider clamps itself)
		//-----------------------------------------------------------------------------
		void SetTextAndSync(int v, bool bTriggerChangeMessage)
		{
			int tv = ClampToBounds(v);
			m_pInputBox->SetText(VarArgs("%d", tv));
			UpdateFromInput(tv, bTriggerChangeMessage);

			// If text value sits outside slider range, base slider may not emit "SliderMoved".
			// Manually emit it so listeners are notified.
			int sMin, sMax; GetRange(sMin, sMax);
			if (tv < sMin || tv > sMax)
			{
				PostActionSignal(new KeyValues("SliderMoved"));
			}
		}

	private:
		TextEntry* m_pInputBox;

		// Optional input bounds (off by default)
		bool m_bUseInputBounds = false;
		int  m_iInputMin = 0;
		int  m_iInputMax = 0;

		//-----------------------------------------------------------------------------
		// Purpose: Allow the input box to change this value
		//-----------------------------------------------------------------------------
		void UpdateFromInput(int iValue, bool bTriggerChangeMessage = true)
		{
			// Move slider thumb within the slider's own range only
			int iMin, iMax;
			GetRange(iMin, iMax);
			int clampedForSlider = clamp(iValue, iMin, iMax);
			BaseClass::SetValue(clampedForSlider, bTriggerChangeMessage);
		}

		//-----------------------------------------------------------------------------
		// Purpose:
		//-----------------------------------------------------------------------------
		int GetInputValue()
		{
			if (m_pInputBox->GetTextLength() == 0)
				return -1;

			// Read the whole number first, then clamp
			char szValue[16] = { 0 };
			m_pInputBox->GetText(szValue, sizeof(szValue));

			if (!szValue[0])
				return -1;

			int iValue = atoi(szValue);

			// Hard cap absolute values and then bounds
			iValue = ClampToBounds(iValue);

			return iValue;
		}

		//-----------------------------------------------------------------------------
		// Purpose: Catch the text box being changed and update the slider
		//-----------------------------------------------------------------------------
		MESSAGE_FUNC_PARAMS(OnTextChanged, "TextChanged", data)
		{
			int iValue = GetInputValue();
			UpdateFromInput(iValue, true);

			// If input exceeds slider range, also notify listeners.
			int iMin, iMax; GetRange(iMin, iMax);
			if (iValue < iMin || iValue > iMax)
			{
				PostActionSignal(new KeyValues("SliderMoved"));
			}
		}

		//-----------------------------------------------------------------------------
		// Purpose: Don't let the box be left on invalid values
		//-----------------------------------------------------------------------------
		MESSAGE_FUNC_PARAMS(OnKillFocus, "TextKillFocus", data)
		{
			int iValue = GetInputValue();

			// Keep what the user typed within TEXT bounds
			int iFinalText = ClampToBounds(iValue);

			m_pInputBox->SetText(VarArgs("%d", iFinalText));
			UpdateFromInput(iFinalText, true);

			// If input exceeds slider range, ensure "SliderMoved" still fires.
			int iMin, iMax; GetRange(iMin, iMax);
			if (iFinalText < iMin || iFinalText > iMax)
			{
				PostActionSignal(new KeyValues("SliderMoved"));
			}
		}
	};
}
#endif
