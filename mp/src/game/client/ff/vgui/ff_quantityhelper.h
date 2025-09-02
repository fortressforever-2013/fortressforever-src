#pragma once
#include "cbase.h"

namespace FFQuantityHelper
{
	template <typename T>
	class optional
	{
	public:
		// types
		typedef T value_type;

		// constructors
		optional()
			: m_ptr(NULL)
		{
		}

		optional(const T& v)
			: m_ptr(new T(v))
		{
		}

		// copy constructor
		optional(const optional& other)
			: m_ptr(other.m_ptr ? new T(*other.m_ptr) : NULL)
		{
		}

		// copy assignment
		optional& operator=(const optional& other)
		{
			if (this != &other)
			{
				if (other.m_ptr)
				{
					if (m_ptr)
						*m_ptr = *other.m_ptr;
					else
						m_ptr = new T(*other.m_ptr);
				}
				else
				{
					reset();
				}
			}
			return *this;
		}

		// destructor
		~optional()
		{
			reset();
		}

		// observers
		bool has_value() const { return m_ptr != NULL; }
		operator bool() const { return has_value(); }

		T& value()
		{
			assert(m_ptr && "optional has no value");
			return *m_ptr;
		}

		const T& value() const
		{
			assert(m_ptr && "optional has no value");
			return *m_ptr;
		}

		T& operator*() { return value(); }
		const T& operator*() const { return value(); }

		T* operator->() { assert(m_ptr); return m_ptr; }
		const T* operator->() const { assert(m_ptr); return m_ptr; }

		// modifiers
		void reset()
		{
			if (m_ptr)
			{
				delete m_ptr;
				m_ptr = nullptr;
			}
		}

		void set(const T& v)
		{
			if (m_ptr)
				*m_ptr = v;
			else
				m_ptr = new T(v);
		}

		void swap(optional& other)
		{
			std::swap(m_ptr, other.m_ptr);
		}

	private:
		T* m_ptr; // NULL = disengaged
	};

	// free swap
	template <typename T>
	inline void swap(optional<T>& a, optional<T>& b)
	{
		a.swap(b);
	}

	struct Scale
	{
		float X, Y;
		Scale() : X(1.0f), Y(1.0f) {}
		Scale(float x, float y) : X(x), Y(y) {}
		bool operator==(const Scale& other) const
		{
			return X == other.X
				&& Y == other.Y;
		}
		bool operator!=(const Scale& other) const
		{
			return !(*this == other);
		}
	};

	struct Offset
	{
		int X, Y;
		Offset() : X(0), Y(0) {}
		Offset(int x, int y) : X(x), Y(y) {}
		bool operator==(const Offset& other) const
		{
			return X == other.X
				&& Y == other.Y;
		}
		bool operator!=(const Offset& other) const
		{
			return !(*this == other);
		}

		Offset Scaled(Scale scale)
		{
			return Offset(
				static_cast<int>(X * scale.X),
				static_cast<int>(Y * scale.Y));
		}
	};

	struct Position
	{
		int X, Y;
		Position() : X(0), Y(0) {}
		Position(int x, int y) : X(x), Y(y) {}
		bool operator==(const Position& other) const
		{
			return X == other.X
				&& Y == other.Y;
		}
		bool operator!=(const Position& other) const
		{
			return !(*this == other);
		}
	};

	struct Size
	{
		int Width, Height;
		Size() : Width(0), Height(0) {}
		Size(int width, int height) : Width(width), Height(height) {}
		bool operator==(const Size& other) const
		{
			return Width == other.Width
				&& Height == other.Height;
		}
		bool operator!=(const Size& other) const
		{
			return !(*this == other);
		}

		Size Scaled(Scale scale)
		{
			return Size(
				static_cast<int>(Width * scale.X),
				static_cast<int>(Height * scale.Y));
		}
	};

	struct Bounds
	{
		float X0, Y0, X1, Y1;
		Bounds()
			: X0(0), Y0(0), X1(0), Y1(0) {}

		Bounds(float x0, float y0, float x1, float y1)
			: X0(x0), Y0(y0), X1(x1), Y1(y1) {}

		Bounds(Position pos, Size size)
			: Bounds(
				pos.X,
				pos.Y,
				pos.X + size.Width,
				pos.Y + size.Height) {}
	};

	enum AmountDisplay {
		DISPLAY_RAW = 0,
		DISPLAY_PERCENTAGE,
		DISPLAY_MAX
	};

	enum AnchorPosition {
		ANCHORPOS_TOPLEFT = 0,
		ANCHORPOS_TOPCENTER,
		ANCHORPOS_TOPRIGHT,
		ANCHORPOS_MIDDLELEFT,
		ANCHORPOS_MIDDLECENTER,
		ANCHORPOS_MIDDLERIGHT,
		ANCHORPOS_BOTTOMLEFT,
		ANCHORPOS_BOTTOMCENTER,
		ANCHORPOS_BOTTOMRIGHT
	};

	enum HorizontalAlignment {
		ALIGN_LEFT = 0,
		ALIGN_CENTER,
		ALIGN_RIGHT
	};

	enum VerticalAlignment {
		ALIGN_TOP = 0,
		ALIGN_MIDDLE,
		ALIGN_BOTTOM
	};

	enum ColorMode {
		COLOR_MODE_CUSTOM = 0,
		COLOR_MODE_STEPPED,
		COLOR_MODE_FADED,
		COLOR_MODE_TEAM
	};

	enum BuildableDisplayOption
	{
		NEVER = 0,
		ALWAYS,
		ON_BUILD,
		IF_BUILT
	};

	template <typename T>
	bool Change(T& existingThing, T newThing)
	{
		bool bHasChange = existingThing != newThing;

		if (bHasChange)
		{
			existingThing = newThing;
		}

		return bHasChange;
	}

	vgui::HFont GetFont(
		vgui::HFont* hfFamily,
		int iSize,
		bool bUseModifier);

	int GetInt(
		const char* keyName,
		KeyValues* kvStyleData,
		int iDefaultValue = -1);

	int GetInt(
		const char* keyName,
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData,
		int iDefaultValue = -1);

	KeyValues* GetData(
		const char* keyName,
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData);

	void CombineHash(
		int& iHash,
		int iValue);

	void ConvertToAlignment(
		AnchorPosition anchorPosition,
		int& iAlignHoriz,
		int& iAlignVert);

	Offset CalculateAnchorOffset(
		Size AnchorSize,
		AnchorPosition anchorPosition);

	Offset CalculatePositionOffset(
		Size anchorSize,
		int iAlignHoriz,
		int iAlignVert);

	Position CalculatePosition(
		AnchorPosition anchorPosition,
		Size anchorSize,
		Size size,
		int horizontalAlignment,
		int verticalAlignment,
		Offset offset);

	const Scale GetScaleFromScreenSize();
}