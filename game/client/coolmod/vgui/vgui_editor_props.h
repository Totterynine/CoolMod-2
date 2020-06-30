#ifndef C_VGUI_EDITOR_PROPS_H
#define C_VGUI_EDITOR_PROPS_H

#include "cbase.h"
#include "vgui_controls/frame.h"


class CVGUIMapaddEditor_Properties : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE( CVGUIMapaddEditor_Properties, vgui::Frame );

public:

	CVGUIMapaddEditor_Properties( vgui::Panel *pParent );
	~CVGUIMapaddEditor_Properties();

	void SendPropertiesToRoot();

	enum PROPERTYMODE
	{
		PROPERTYMODE_ENTITY = 0,
	};

	PROPERTYMODE GetPropertyMode();
	void SetPropertyMode( PROPERTYMODE mode );
	void OnRequestPropertyLayout( PROPERTYMODE mode );

protected:

	void OnSizeChanged( int newWide, int newTall );

	void PerformLayout();

	MESSAGE_FUNC_PTR( OnPropertyChanged, "PropertyChanged", panel );
	MESSAGE_FUNC( OnLightSelectionChanged, "LightSelectionChanged" );
	MESSAGE_FUNC( OnFileLoaded, "FileLoaded" );

private:

	PROPERTYMODE m_iPropMode;

	vgui::PanelListPanel *m_pListRoot;

	CUtlVector< Panel* > m_hProperties;

	void FlushProperties();
	void UpdatePropertyVisibility();

	void CreateProperties();
	void CreateProperties_Entity();
	void AddPropertyToList( Panel *panel );

	void PropertiesReadAll();
	void RefreshGlobalLightData();

	KeyValues *AllocKVFromVisible();
};


#endif