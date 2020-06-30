#ifndef C_MAPADD_EDITOR_H
#define C_MAPADD_EDITOR_H

#include "cbase.h"

enum ENT_HULL_ID
{
	HULL_HUMAN,				// Combine, Stalker, Zombie...
	HULL_SMALL_CENTERED,	// Scanner
	HULL_WIDE_HUMAN,		// Vortigaunt
	HULL_TINY,				// Headcrab
	HULL_WIDE_SHORT,		// Bullsquid
	HULL_MEDIUM,			// Cremator
	HULL_TINY_CENTERED,		// Manhack 
	HULL_LARGE,				// Antlion Guard
	HULL_LARGE_CENTERED,	// Mortar Synth
	HULL_MEDIUM_TALL,		// Hunter
	//--------------------------------------------
	NUM_HULLS,
	HULL_NONE				// No Hull (appears after num hulls as we don't want to count it)
};

struct mapadd_ent_t
{
	DECLARE_DATADESC();

	char name[128];
	Vector pos;
	QAngle ang;
	ENT_HULL_ID hull;
	KeyValues* pKV;
	
	static mapadd_ent_t* AllocateFromKeyValues(KeyValues* pKV);
	KeyValues* AllocateAsKeyValues();
	void ApplyKeyValueProperties(KeyValues* pKV);
};

enum ENT_PARAM_ID
{
	EPARAM_ORIGIN = 0,
	EPARAM_ANGLE,
	EPARAM_NAME,

	EPARAM_COUNT,
};
const char* GetEntityParamName(ENT_PARAM_ID id);

ENT_HULL_ID GetHullIDFromName(const char *name);
void GetEntityHullFromID(ENT_PARAM_ID id, Vector &mins, Vector &maxs);

class mapadd_editor_t : public mapadd_ent_t
{
public:
	mapadd_editor_t();
	int iEditorId;
};

#define POINT_BOX_SIZE 8.0f
#define LIGHT_BOX_VEC Vector( POINT_BOX_SIZE, POINT_BOX_SIZE, POINT_BOX_SIZE )

#define HELPER_COLOR_MAX 0.8f
#define HELPER_COLOR_MIN 0.1f
#define HELPER_COLOR_LOW 0.4f
#define SELECTED_AXIS_COLOR Vector( HELPER_COLOR_MAX, HELPER_COLOR_MAX, HELPER_COLOR_MIN )

#define SELECTION_PICKER_SIZE 2.0f
#define SELECTION_PICKER_VEC Vector( SELECTION_PICKER_SIZE, SELECTION_PICKER_SIZE, SELECTION_PICKER_SIZE )

class CMapaddEditor : public CAutoGameSystemPerFrame
{
	typedef CAutoGameSystemPerFrame BaseClass;
public:

	CMapaddEditor();
	~CMapaddEditor();

	bool Init();

	void LevelInitPostEntity();
	void LevelShutdownPreEntity();

	void Update( float ft );

	void OnRender();

	void SetEditorActive( bool bActive, bool bView = true, bool bLights = true );
	bool IsEditorActive();
	bool IsEditorLightingActive();

	bool IsLightingEditorAllowed();

	void GetEditorView( Vector *origin, QAngle *angles );
	void SetEditorView( const Vector *origin, const QAngle *angles );

	Vector &GetMoveDirForModify();
	void AbortEditorMovement( bool bStompVelocity );

	KeyValues *VmfToKeyValues( const char *pszVmf );
	void KeyValuesToVmf( KeyValues *pKV, CUtlBuffer &vmf );

	void LoadVmf( const char *pszVmf );
	void SaveCurrentVmf();
	const char *GetCurrentVmfPath();

	enum EDITORINTERACTION_MODE
	{
		EDITORINTERACTION_SELECT = 0,
		EDITORINTERACTION_ADD,
		EDITORINTERACTION_TRANSLATE,
		EDITORINTERACTION_ROTATE,
		EDITORINTERACTION_COPY,

		EDITORINTERACTION_COUNT,
	};

	void SetEditorInteractionMode( EDITORINTERACTION_MODE mode );
	EDITORINTERACTION_MODE GetEditorInteractionMode();

	enum EDITOR_SELECTEDAXIS
	{
		EDITORAXIS_NONE = 0,
		EDITORAXIS_X,
		EDITORAXIS_Y,
		EDITORAXIS_Z,
		EDITORAXIS_SCREEN,
		EDITORAXIS_PLANE_XY,
		EDITORAXIS_PLANE_XZ,
		EDITORAXIS_PLANE_YZ,

		EDITORAXIS_COUNT,
		EDITORAXIS_FIRST = EDITORAXIS_X,
		EDITORAXIS_FIRST_PLANE = EDITORAXIS_PLANE_XY,
	};

	EDITOR_SELECTEDAXIS GetCurrentSelectedAxis();
	void SetCurrentSelectedAxis( EDITOR_SELECTEDAXIS axis );
	void UpdateCurrentSelectedAxis( int x, int y );

	enum EDITOR_DBG_MODES
	{
		EDITOR_DBG_OFF = 0,
		EDITOR_DBG_LIGHTING,
		EDITOR_DBG_DEPTH,
		EDITOR_DBG_NORMALS,
	};

	EDITOR_DBG_MODES GetDebugMode();
	void SetDebugMode( EDITOR_DBG_MODES mode );

	void AddEntityFromScreen( int x, int y, KeyValues *pData );
	void CopyEntityFromScreen(int x, int y);
	bool SelectEntity( int x, int y, bool bGroup, bool bToggle );
	bool SelectEntities( int *pi2_BoundsMin, int *pi2_BoundsMax,
		bool bGroup, bool bToggle );
	void MoveSelectedEntities( Vector delta );
	void RotateSelectedEntities( VMatrix matRotate );

	void ClearSelection();
	bool IsEntitySelected(mapadd_ent_t*l );
	void DeleteSelectedLights();
	void SetSelectionCenterLocked( bool locked );

	KeyValues *GetKVFromLight(mapadd_ent_t*pLight );
	void GetKVFromAll( CUtlVector< KeyValues* > &listOut );
	void GetKVFromSelection( CUtlVector< KeyValues* > &listOut );
	void ApplyKVToSelection( KeyValues *pKVChanges );

	int GetNumSelected();
	bool IsAnythingSelected();
	Vector GetSelectionCenter();
	QAngle GetSelectionOrientation();

	Vector GetViewOrigin();
	QAngle GetViewAngles();
	Vector GetPickingRay( int x, int y );

	const mapadd_editor_t&GetGlobalState();

private:

	bool m_bActive;
	bool m_bLightsActive;

	bool m_bSelectionCenterLocked;
	Vector m_vecSelectionCenterCache;

	void FlushEditorLights();

	void AddEditorLight(mapadd_ent_t*pDef );

	void AddEntityToSelection(mapadd_ent_t*l );
	void RemoveEntityFromSelection(mapadd_ent_t*l );

	char m_szCurrentMapadd[MAX_PATH*4];
	void ParseMapaddFile( KeyValues *pKeyValues );
	void ApplyEntitiesToCurrentMapaddFile();

	CUtlVector< mapadd_ent_t* > m_hEditorEnts;
	CUtlVector< mapadd_ent_t* > m_hSelectedEnts;

	KeyValues *m_pKVMA;

	Vector m_vecEditorView_Origin;
	QAngle m_angEditorView_Angles;
	Vector m_vecMoveDir;
	Vector m_vecEditorView_Velocity;

	EDITORINTERACTION_MODE m_iInteractionMode;

	CMaterialReference m_matSprite;
	CMaterialReference m_matSelect;
	CMaterialReference m_matHelper;

	EDITOR_SELECTEDAXIS m_iCurrentSelectedAxis;
	EDITOR_DBG_MODES m_iDebugMode;

	void RenderSprites();
	void RenderSelection();

	void RenderHelpers();
	void RenderTranslate();
	void RenderRotate();
};

CMapaddEditor *GetMapaddEditor();

#endif