#ifndef FF_HUD_STATUS_MANCANNON_AVAILABILITY_H
#define FF_HUD_STATUS_MANCANNON_AVAILABILITY_H

// Tracks the availability last sent to this player's HUD.
struct FFHudStatusManCannonAvailability
{
	bool IsUnchanged(bool bCanHave, bool bHas) const
	{
		return m_bSent && m_bCanHave == bCanHave && m_bHas == bHas;
	}

	void Remember(bool bCanHave, bool bHas)
	{
		m_bCanHave = bCanHave;
		m_bHas = bHas;
		m_bSent = true;
	}

	void Invalidate() { m_bSent = false; }

private:
	bool m_bSent = false;
	bool m_bCanHave = false;
	bool m_bHas = false;
};

#endif
