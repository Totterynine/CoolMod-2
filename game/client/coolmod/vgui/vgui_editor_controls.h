#ifndef C_VGUI_EDITOR_CONTROLS_H
#define C_VGUI_EDITOR_CONTROLS_H

#include "cbase.h"
#include "vgui_controls/frame.h"


class CVGUIMapaddEditor_Controls : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE( CVGUIMapaddEditor_Controls, vgui::Frame );

public:

	CVGUIMapaddEditor_Controls( vgui::Panel *pParent );
	~CVGUIMapaddEditor_Controls();

protected:

	void PerformLayout();

	MESSAGE_FUNC( OnLevelSpawn, "LevelSpawn" );

	MESSAGE_FUNC_PTR( OnRadioButtonChecked, "RadioButtonChecked", panel );
	MESSAGE_FUNC_PTR( OnCheckButtonChecked, "CheckButtonChecked", panel );
	MESSAGE_FUNC_PTR( OnTextChanged, "TextChanged", panel );

	MESSAGE_FUNC_PARAMS( OnFileSelected, "FileSelected", pKV );

	void OnCommand( const char *pCmd );

private:

	void OnLoadVmf();
	void LoadVmf( const char *pszPath );

	void BuildVmfPath( char *pszOut, int maxlen, bool bMakeRelative = true );
	bool BuildCurrentVmfPath( char *pszOut, int maxlen );
	void OpenVmfFileDialog();
	vgui::FileOpenDialog *m_pFileVmf;
	vgui::DirectorySelectDialog *m_pDirVmf;
};

#endif