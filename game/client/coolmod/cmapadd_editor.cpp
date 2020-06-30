
#include "cbase.h"
#include "cmapadd_editor.h"
#include "iviewrender.h"
#include "view_shared.h"
#include "view.h"
#include "debugoverlay_shared.h"
#include "collisionutils.h"
#include "beamdraw.h"

#include "filesystem.h"

extern void ScreenToWorld( int mousex, int mousey, float fov,
					const Vector& vecRenderOrigin,
					const QAngle& vecRenderAngles,
					Vector& vecPickingRay );

static Vector* PointsFromMinMax(Vector vMin, Vector vMax)
{
	Vector* points = new Vector[8];
	points[0] = Vector(vMax.x, vMax.y, vMax.z);
	points[1] = Vector(vMin.x, vMax.y, vMax.z);
	points[2] = Vector(vMax.x, vMin.y, vMax.z);
	points[3] = Vector(vMax.x, vMax.y, vMin.z);
	points[4] = Vector(vMin.x, vMin.y, vMax.z);
	points[5] = Vector(vMax.x, vMin.y, vMin.z);
	points[6] = Vector(vMin.x, vMax.y, vMin.z);
	points[7] = Vector(vMin.x, vMin.y, vMin.z);
	return points;
}

static const char* g_pszLightParamNames[EPARAM_COUNT] =
{
	"origin",
	"angle",
	"classname"
};

const char* GetLightParamName(ENT_PARAM_ID id)
{
	Assert(id >= 0 && id < LPARAM_COUNT);

	return g_pszLightParamNames[id];
}


BEGIN_DATADESC_NO_BASE(mapadd_ent_t)
DEFINE_KEYFIELD(pos, FIELD_VECTOR, GetLightParamName(EPARAM_ORIGIN)),
DEFINE_KEYFIELD(ang, FIELD_VECTOR, GetLightParamName(EPARAM_ANGLE)),
DEFINE_KEYFIELD(name, FIELD_STRING, GetLightParamName(EPARAM_NAME)),
END_DATADESC()

mapadd_ent_t* mapadd_ent_t::AllocateFromKeyValues(KeyValues* pKV)
{
	mapadd_ent_t* pRet = new mapadd_ent_t();

	pRet->ApplyKeyValueProperties(pKV);

	return pRet;
}

KeyValues* mapadd_ent_t::AllocateAsKeyValues()
{
	typedescription_t* dt = GetDataDescMap()->dataDesc;
	int iNumFields = GetDataDescMap()->dataNumFields;

	KeyValues* pRet = new KeyValues(name);

	for (int iField = 0; iField < iNumFields; iField++)
	{
		typedescription_t& pField = dt[iField];
		const char* pszFieldName = pField.externalName;
		if (!Q_stricmp(pszFieldName, GetLightParamName(EPARAM_ORIGIN)) ||
			!Q_stricmp(pszFieldName, GetLightParamName(EPARAM_ANGLE)))
		{
			Vector pos;
			Q_memcpy(pos.Base(), (void*)((int)this + (int)pField.fieldOffset[0]), pField.fieldSizeInBytes);

			pRet->SetString(pszFieldName, VarArgs("%.2f %.2f %.2f", XYZ(pos)));
		}
		else if (!Q_stricmp(pszFieldName, GetLightParamName(EPARAM_NAME)))
		{
			pRet->SetString(pszFieldName, name);
		}
	}

	return pRet;
}

void mapadd_ent_t::ApplyKeyValueProperties(KeyValues* pKV)
{
	typedescription_t* dt = GetDataDescMap()->dataDesc;
	int iNumFields = GetDataDescMap()->dataNumFields;

	for (KeyValues* pValue = pKV->GetFirstValue(); pValue; pValue = pValue->GetNextValue())
	{
		const char* pszKeyName = pValue->GetName();

		if (!pszKeyName || !*pszKeyName)
			continue;

		for (int iField = 0; iField < iNumFields; iField++)
		{
			typedescription_t& pField = dt[iField];

			if (!Q_stricmp(pField.externalName, pszKeyName))
			{
				if (!Q_stricmp(pszKeyName, GetLightParamName(EPARAM_ORIGIN)) ||
					!Q_stricmp(pszKeyName, GetLightParamName(EPARAM_ANGLE)))
				{
					float f[3] = { 0 };
					UTIL_StringToFloatArray(f, 3, pValue->GetString());
					Q_memcpy((void*)((int)this + (int)pField.fieldOffset[0]), f, pField.fieldSizeInBytes);
				}
				else if (!Q_stricmp(pszKeyName, GetLightParamName(EPARAM_NAME)))
				{
					//Q_memcpy((void*)name, pValue->GetString(), Q_strlen(pValue->GetString()));
					Q_strcpy(name, pValue->GetString());
					hull = GetHullIDFromName(name);
				}

				break;
			}
		}
	}
}

// god forgive me for i have sinned 
ENT_HULL_ID GetHullIDFromName(const char* name)
{
	if (!Q_strcmp(name, "npc_metropolice") ||
		!Q_strcmp(name, "npc_combine_s") ||
		!Q_strcmp(name, "npc_citizen") ||
		!Q_strcmp(name, "npc_alyx") ||
		!Q_strcmp(name, "npc_barney") ||
		!Q_strcmp(name, "npc_eli") ||
		!Q_strcmp(name, "npc_mossman") ||
		!Q_strcmp(name, "npc_breen") ||
		!Q_strcmp(name, "npc_gman") ||
		!Q_strcmp(name, "npc_grigori") ||
		!Q_strcmp(name, "npc_vortigaunt"))
	{
		return HULL_HUMAN;
	}
	else if (!Q_strcmp(name, "npc_antlion"))
	{
		return HULL_MEDIUM;
	}
	else if (!Q_strcmp(name, "npc_scanner") ||
		!Q_strcmp(name, "npc_rollermine"))
	{
		return HULL_SMALL_CENTERED;
	}
	else if (!Q_strcmp(name, "npc_headcrab") ||
		!Q_strcmp(name, "npc_headcrab_fast") ||
		!Q_strcmp(name, "npc_headcrab_poison") ||
		!Q_strcmp(name, "npc_headcrab_black") ||
		!Q_strcmp(name, "npc_crow") ||
		!Q_strcmp(name, "npc_seagull") ||
		!Q_strcmp(name, "npc_pigeon")		
		)
	{
		return HULL_TINY;
	}
	else if (!Q_strcmp(name, "npc_antlionguard"))
	{
		return HULL_LARGE;
	}
	else if (!Q_strcmp(name, "npc_helicopter") ||
		!Q_strcmp(name, "npc_advisor") ||
		!Q_strcmp(name, "npc_combinegunship") ||
		!Q_strcmp(name, "npc_ichthyosaur") ||
		!Q_strcmp(name, "npc_strider"))
	{
		return HULL_LARGE_CENTERED;
	}
	else
	{
		return HULL_NONE;
	}
}

void GetEntityHullFromID(ENT_HULL_ID id, Vector& mins, Vector& maxs)
{
	switch (id)
	{
	case HULL_HUMAN:
		mins = Vector(-13, -13, 0);
		maxs = Vector(13, 13, 72);
		break;
	case HULL_SMALL_CENTERED:
		mins = Vector(-20, -20, 20);
		maxs = Vector(12, 12, 12);
		break;
	case HULL_TINY:
		mins = Vector(-12, -12, 0);
		maxs = Vector(12, 12, 24);
		break;
	case HULL_LARGE:
		mins = Vector(-40, -40, 0);
		maxs = Vector(40, 40, 120);
		break;
	case HULL_LARGE_CENTERED:
		mins = Vector(-38, -38, -38);
		maxs = Vector(38, 38, 38);
		break;
	default:
		mins = -LIGHT_BOX_VEC;
		maxs = LIGHT_BOX_VEC;
		break;
	}

	return;
}

static CMapaddEditor __g_lightingEditor;
CMapaddEditor * GetMapaddEditor()
{
	return &__g_lightingEditor;
}

mapadd_editor_t::mapadd_editor_t()
{
	iEditorId = -1;
}


CMapaddEditor::CMapaddEditor()
	: BaseClass( "MapaddEditorSystem" )
{
	m_bActive = false;
	m_bLightsActive = false;
	m_bSelectionCenterLocked = false;

	m_iDebugMode = EDITOR_DBG_OFF;

	m_pKVMA = NULL;
	*m_szCurrentMapadd = 0;

	m_vecEditorView_Origin = vec3_origin;
	m_angEditorView_Angles = vec3_angle;

	m_vecEditorView_Velocity = vec3_origin;
	m_vecMoveDir = vec3_origin;
	m_vecSelectionCenterCache.Init();

	m_iInteractionMode = EDITORINTERACTION_SELECT;
	m_iCurrentSelectedAxis = EDITORAXIS_NONE;
}

CMapaddEditor::~CMapaddEditor()
{
	if ( m_pKVMA )
		m_pKVMA->deleteThis();
}

int DefLightSort( mapadd_ent_t *const *d0, mapadd_ent_t *const *d1 )
{
	Vector vec0 = ( *d0 )->pos;
	Vector vec1 = ( *d1 )->pos;
	Vector view = CurrentViewOrigin();

	float dist0 = (view-vec0).LengthSqr();
	float dist1 = (view-vec1).LengthSqr();

	return ( dist0 > dist1 ) ? -1 : 1;
}

bool CMapaddEditor::Init()
{
	m_matSprite.Init( "mapaddedit/editor_sprite", TEXTURE_GROUP_OTHER );
	Assert( m_matSprite.IsValid() );

	m_matSelect.Init( "mapaddedit/editor_select", TEXTURE_GROUP_OTHER );
	Assert( m_matSelect.IsValid() );

	m_matHelper.Init( "mapaddedit/editor_helper", TEXTURE_GROUP_OTHER );
	Assert( m_matHelper.IsValid() );

	return true;
}

void CMapaddEditor::LevelInitPostEntity()
{
}

void CMapaddEditor::LevelShutdownPreEntity()
{
	SetEditorActive( false );

	FlushEditorLights();

	m_iCurrentSelectedAxis = EDITORAXIS_NONE;
	SetDebugMode( EDITOR_DBG_OFF );
}

bool CMapaddEditor::IsLightingEditorAllowed()
{
	//static ConVarRef cvarCheats( "sv_cheats" );
	//return cvarCheats.GetBool();

	return true;
}

void CMapaddEditor::Update( float ft )
{
	if ( !IsEditorActive() )
		return;

	if ( !IsLightingEditorAllowed() )
	{
		SetEditorActive( false );
	}

	engine->Con_NPrintf(6, "ENTITIES: %i", m_hEditorEnts.Count());

	Vector fwd, right;
	AngleVectors( m_angEditorView_Angles, &fwd, &right, NULL );

	float flIdealSpeed = ( 300.0f + m_vecMoveDir.z * 400.0f );
	Vector vecIdealMove = fwd * m_vecMoveDir.x + right * m_vecMoveDir.y;
	vecIdealMove *= flIdealSpeed;

	Vector vecDelta = vecIdealMove - m_vecEditorView_Velocity;
	if ( !vecDelta.IsZero() )
		m_vecEditorView_Velocity += vecDelta * ft * 10.0f;

	if ( m_vecEditorView_Velocity.LengthSqr() > 1 )
		m_vecEditorView_Origin += m_vecEditorView_Velocity * ft;
}

void CMapaddEditor::SetSelectionCenterLocked( bool locked )
{
	if ( locked )
		m_vecSelectionCenterCache = GetSelectionCenter();

	m_bSelectionCenterLocked = locked;
}

CMapaddEditor::EDITOR_DBG_MODES CMapaddEditor::GetDebugMode()
{
	return m_iDebugMode;
}

void CMapaddEditor::SetDebugMode( EDITOR_DBG_MODES mode )
{
	m_iDebugMode = mode;
}

CMapaddEditor::EDITOR_SELECTEDAXIS CMapaddEditor::GetCurrentSelectedAxis()
{
	return m_iCurrentSelectedAxis;
}

void CMapaddEditor::SetCurrentSelectedAxis( EDITOR_SELECTEDAXIS axis )
{
	m_iCurrentSelectedAxis = axis;
}

void CMapaddEditor::OnRender()
{
	if ( !IsEditorActive() )
		return;

	RenderSprites();

	CMatRenderContextPtr pRenderContext(materials);
	float savedNear, savedFar;
	savedNear = 0.0f;
	savedFar = 1.0f;
	pRenderContext->DepthRange(0.0f, 0.1f);

	RenderSelection();

	RenderHelpers();
	pRenderContext->DepthRange(savedNear, savedFar);
}

void CMapaddEditor::RenderSprites()
{
	CMatRenderContextPtr pRenderContext( materials );
	pRenderContext->Bind(m_matSelect);

	CUtlVector< mapadd_ent_t* > hSortedLights;

	if (!m_hSelectedEnts.IsEmpty())
	{
		FOR_EACH_VEC(m_hEditorEnts, i)
		{
			if (m_hSelectedEnts.HasElement(m_hEditorEnts[i]))
				continue;

			hSortedLights.AddToTail(m_hEditorEnts[i]);
		}
	}
	else
	{
		hSortedLights.AddVectorToTail(m_hEditorEnts);
	}

	hSortedLights.Sort( DefLightSort );

	IMesh* pMesh = pRenderContext->GetDynamicMesh(true, NULL, NULL, m_matSelect);

	CMeshBuilder meshBuilder;
	meshBuilder.Begin(pMesh, MATERIAL_QUADS, 6 * hSortedLights.Count());

	FOR_EACH_VEC(hSortedLights, i)
	{
		mapadd_ent_t* l = hSortedLights[i];

		Vector position = l->pos;
		Vector color(0.5f, 1.0f, 0.25f);

		Vector mins, maxs;
		GetEntityHullFromID(l->hull, mins, maxs);
		Vector* points = PointsFromMinMax(mins, maxs);

		matrix3x4_t rotation;
		AngleMatrix(l->ang, rotation);

		for (int p = 0; p < 8; p++)
			VectorRotate(points[p], rotation, points[p]);

		for (int p = 0; p < 8; p++)
			points[p] += position;

		int iPlaneSetup[6][4] = {
			{7,5,3,6},
			{0,3,5,2},
			{1,4,7,6},
			{0,1,6,3},
			{2,5,7,4},
			{0,2,4,1},
		};

		Vector2D uvs[4] = {
			Vector2D(0, 0),
			Vector2D(1, 0),
			Vector2D(1, 1),
			Vector2D(0, 1),
		};

		for (int p = 0; p < 6; p++)
		{
			for (int v = 0; v < 4; v++)
			{
				meshBuilder.Position3fv(points[iPlaneSetup[p][v]].Base());
				meshBuilder.Color3fv(color.Base());
				meshBuilder.TexCoord2fv(0, uvs[v].Base());
				meshBuilder.AdvanceVertex();
			}
		}

		free(points);
	}

	meshBuilder.End();
	pMesh->Draw();
}

void CMapaddEditor::RenderSelection()
{
	CMatRenderContextPtr pRenderContext( materials );
	pRenderContext->Bind( m_matSelect );

	CUtlVector< mapadd_ent_t* > hSelectedLights;
	hSelectedLights.AddVectorToTail( m_hSelectedEnts );
	hSelectedLights.Sort( DefLightSort );

	IMesh *pMesh = pRenderContext->GetDynamicMesh( true, NULL, NULL, m_matSelect );

	CMeshBuilder meshBuilder;
	meshBuilder.Begin( pMesh, MATERIAL_QUADS, 6 * hSelectedLights.Count() );

	FOR_EACH_VEC( hSelectedLights, i )
	{
		mapadd_ent_t *l = hSelectedLights[ i ];

		Vector position = l->pos;
		Vector color(SELECTED_AXIS_COLOR);

		Vector mins, maxs;
		GetEntityHullFromID(l->hull, mins, maxs);
		Vector *points = PointsFromMinMax(mins, maxs);

		matrix3x4_t rotation;
		AngleMatrix(l->ang, rotation);

		for (int p = 0; p < 8; p++)
			VectorRotate(points[p], rotation, points[p]);

		for ( int p = 0; p < 8; p++ )
			points[p] += position;

		int iPlaneSetup[6][4] = {
			{7,5,3,6},
			{0,3,5,2},
			{1,4,7,6},
			{0,1,6,3},
			{2,5,7,4},
			{0,2,4,1},
		};

		Vector2D uvs[4] = {
			Vector2D( 0, 0 ),
			Vector2D( 1, 0 ),
			Vector2D( 1, 1 ),
			Vector2D( 0, 1 ),
		};

		for ( int p = 0; p < 6; p++ )
		{
			for ( int v = 0; v < 4; v++ )
			{
				meshBuilder.Position3fv( points[iPlaneSetup[p][v]].Base() );
				meshBuilder.Color3fv( color.Base() );
				meshBuilder.TexCoord2fv( 0, uvs[v].Base() );
				meshBuilder.AdvanceVertex();
			}
		}

		free(points);
	}

	meshBuilder.End();
	pMesh->Draw();
}

void CMapaddEditor::RenderHelpers()
{
	if ( !IsAnythingSelected() )
		return;

	switch ( GetEditorInteractionMode() )
	{
	case EDITORINTERACTION_TRANSLATE:
			RenderTranslate();
		break;
	case EDITORINTERACTION_ROTATE:
			RenderRotate();
		break;
	}
}

void CMapaddEditor::RenderTranslate()
{
	CMatRenderContextPtr pRenderContext( materials );
	pRenderContext->Bind( m_matHelper );

	const bool bMultiSelected = (GetNumSelected() > 1);

	const int iSubDiv = 16;
	const float flLengthBase = 16.0f + ( bMultiSelected ? POINT_BOX_SIZE : 0 );
	const float flLengthTip = 8.0f;
	const float flRadiusBase = 1.2f;
	const float flRadiusTip = 4.0f;
	const float flAngleStep = 360.0f / iSubDiv;
	const float flOffset = bMultiSelected ? 0 : POINT_BOX_SIZE;

	Vector directions[] = {
		Vector( 1, 0, 0 ),
		Vector( 0, 1, 0 ),
		Vector( 0, 0, 1 ),
	};

	Vector colors[] = {
		Vector( HELPER_COLOR_MAX, HELPER_COLOR_MIN, HELPER_COLOR_MIN ),
		Vector( HELPER_COLOR_MIN, HELPER_COLOR_MAX, HELPER_COLOR_MIN ),
		Vector( HELPER_COLOR_MIN, HELPER_COLOR_MIN, HELPER_COLOR_MAX ),
	};

	const float flColorLowScale = HELPER_COLOR_LOW;

	Vector center = GetSelectionCenter();

	for ( int iDir = 0; iDir < 3; iDir++ )
	{
		Vector fwd = directions[ iDir ];
		QAngle ang;

		VectorAngles( fwd, ang );

		IMesh *pMesh = pRenderContext->GetDynamicMesh( true, NULL, NULL, m_matHelper );

		CMeshBuilder mB;
		mB.Begin( pMesh, MATERIAL_TRIANGLES, 5 * iSubDiv );

		Vector cUp, nUp;
		Vector pos, tmp0, tmp1, tmp2;

		Vector colHigh = colors[ iDir ];

		if ( GetCurrentSelectedAxis() == EDITORAXIS_FIRST + iDir )
			colHigh = SELECTED_AXIS_COLOR;

		Vector colLow = colHigh * flColorLowScale;

		for ( int iStep = 0; iStep < iSubDiv; iStep++ )
		{
			QAngle angNext( ang );
			angNext.z += flAngleStep;

			AngleVectors( ang, NULL, NULL, &cUp );
			AngleVectors( angNext, NULL, NULL, &nUp );

			// disc end
			pos = center + fwd * flOffset;
			mB.Position3fv( pos.Base() );
			mB.TexCoord2f( 0, 0, 0 );
			mB.Color3fv( colLow.Base() );
			mB.AdvanceVertex();

			tmp0 = pos + cUp * flRadiusBase;
			mB.Position3fv( tmp0.Base() );
			mB.TexCoord2f( 0, 0, 0 );
			mB.Color3fv( colLow.Base() );
			mB.AdvanceVertex();

			tmp1 = pos + nUp * flRadiusBase;
			mB.Position3fv( tmp1.Base() );
			mB.TexCoord2f( 0, 0, 0 );
			mB.Color3fv( colLow.Base() );
			mB.AdvanceVertex();

			// base cylinder
			mB.Position3fv( tmp1.Base() );
			mB.TexCoord2f( 0, 0, 0 );
			mB.Color3fv( colHigh.Base() );
			mB.AdvanceVertex();

			mB.Position3fv( tmp0.Base() );
			mB.TexCoord2f( 0, 0, 0 );
			mB.Color3fv( colHigh.Base() );
			mB.AdvanceVertex();

			tmp2 = tmp0 + fwd * flLengthBase;
			mB.Position3fv( tmp2.Base() );
			mB.TexCoord2f( 0, 0, 0 );
			mB.Color3fv( colHigh.Base() );
			mB.AdvanceVertex();

			mB.Position3fv( tmp1.Base() );
			mB.TexCoord2f( 0, 0, 0 );
			mB.Color3fv( colHigh.Base() );
			mB.AdvanceVertex();

			mB.Position3fv( tmp2.Base() );
			mB.TexCoord2f( 0, 0, 0 );
			mB.Color3fv( colHigh.Base() );
			mB.AdvanceVertex();

			tmp1 += fwd * flLengthBase;
			mB.Position3fv( tmp1.Base() );
			mB.TexCoord2f( 0, 0, 0 );
			mB.Color3fv( colHigh.Base() );
			mB.AdvanceVertex();

			// disc mid
			pos += fwd * flLengthBase;
			mB.Position3fv( pos.Base() );
			mB.TexCoord2f( 0, 0, 0 );
			mB.Color3fv( colLow.Base() );
			mB.AdvanceVertex();

			tmp0 = pos + cUp * flRadiusTip;
			mB.Position3fv( tmp0.Base() );
			mB.TexCoord2f( 0, 0, 0 );
			mB.Color3fv( colLow.Base() );
			mB.AdvanceVertex();

			tmp1 = pos + nUp * flRadiusTip;
			mB.Position3fv( tmp1.Base() );
			mB.TexCoord2f( 0, 0, 0 );
			mB.Color3fv( colLow.Base() );
			mB.AdvanceVertex();

			// tip
			mB.Position3fv( tmp1.Base() );
			mB.TexCoord2f( 0, 0, 0 );
			mB.Color3fv( colHigh.Base() );
			mB.AdvanceVertex();

			mB.Position3fv( tmp0.Base() );
			mB.TexCoord2f( 0, 0, 0 );
			mB.Color3fv( colHigh.Base() );
			mB.AdvanceVertex();

			pos += fwd * flLengthTip;
			mB.Position3fv( pos.Base() );
			mB.TexCoord2f( 0, 0, 0 );
			mB.Color3fv( colHigh.Base() );
			mB.AdvanceVertex();

			ang = angNext;
		}

		mB.End();
		pMesh->Draw();

		if ( GetCurrentSelectedAxis() == EDITORAXIS_FIRST_PLANE + iDir )
		{
			const int index = GetCurrentSelectedAxis() - EDITORAXIS_FIRST_PLANE;
			const float flPlaneSize = POINT_BOX_SIZE + 6.0f;

			int orient[3][2] = {
				{ 0, 1 },
				{ 2, 0 },
				{ 1, 2 },
			};

			int axis_0 = orient[ index ][ 0 ];
			int axis_1 = orient[ index ][ 1 ];

			Vector points[4];
			points[0] = center;
			points[1] = center;
			points[2] = center;
			points[3] = center;

			points[1][axis_0] += flPlaneSize;
			points[2][axis_0] += flPlaneSize;
			points[2][axis_1] += flPlaneSize;
			points[3][axis_1] += flPlaneSize;

			Vector col = SELECTED_AXIS_COLOR;

			mB.Begin( pMesh, MATERIAL_QUADS, 2 );

			for ( int i = 3; i >= 0; i-- )
			{
				mB.Position3fv( points[i].Base() );
				mB.Color3fv( col.Base() );
				mB.TexCoord2f( 0, 0, 0 );
				mB.AdvanceVertex();
			}

			col *= HELPER_COLOR_LOW;

			for ( int i = 0; i < 4; i++ )
			{
				mB.Position3fv( points[i].Base() );
				mB.Color3fv( col.Base() );
				mB.TexCoord2f( 0, 0, 0 );
				mB.AdvanceVertex();
			}

			mB.End();
			pMesh->Draw();
		}
		else
		{
			const int index = iDir;
			const float flPlaneSize = POINT_BOX_SIZE + 6.0f;

			int orient[3][2] = {
				{ 0, 1 },
				{ 2, 0 },
				{ 1, 2 },
			};

			int axis_0 = orient[index][0];
			int axis_1 = orient[index][1];

			Vector points[4];
			points[0] = center;
			points[1] = center;
			points[2] = center;
			points[3] = center;

			points[0][axis_0] += flPlaneSize / 2;
			points[0][axis_1] += flPlaneSize / 2;

			points[1][axis_0] += flPlaneSize;
			points[1][axis_1] += flPlaneSize / 2;
			points[2][axis_0] += flPlaneSize;
			points[2][axis_1] += flPlaneSize;
			points[3][axis_0] += flPlaneSize / 2;
			points[3][axis_1] += flPlaneSize;

			Vector col = (colors[orient[index][0]] + colors[orient[index][1]]);

			mB.Begin(pMesh, MATERIAL_QUADS, 2);

			for (int i = 3; i >= 0; i--)
			{
				mB.Position3fv(points[i].Base());
				mB.Color3fv(col.Base());
				mB.TexCoord2f(0, 0, 0);
				mB.AdvanceVertex();
			}

			col *= HELPER_COLOR_LOW;

			for (int i = 0; i < 4; i++)
			{
				mB.Position3fv(points[i].Base());
				mB.Color3fv(col.Base());
				mB.TexCoord2f(0, 0, 0);
				mB.AdvanceVertex();
			}

			mB.End();
			pMesh->Draw();
		}
	}
}

void CMapaddEditor::RenderRotate()
{
	CMatRenderContextPtr pRenderContext( materials );
	pRenderContext->Bind( m_matHelper );

	const int iSubDiv = 64.0f;
	const float flRadius = 32.0f;
	const float flThickness = 4.0f;
	const float flRotationStep = 360.0f / iSubDiv;

	Vector center = GetSelectionCenter();
	QAngle orientation = GetSelectionOrientation();

	Vector colors[] = {
		Vector( HELPER_COLOR_MAX, HELPER_COLOR_MIN, HELPER_COLOR_MIN ),
		Vector( HELPER_COLOR_MIN, HELPER_COLOR_MAX, HELPER_COLOR_MIN ),
		Vector( HELPER_COLOR_MIN, HELPER_COLOR_MIN, HELPER_COLOR_MAX ),
	};
	const float flColorLowScale = HELPER_COLOR_LOW;

	Vector colHigh, colLow;
	Vector tmp[4];

	Vector reinterpretRoll[3];

	AngleVectors( orientation,
		&reinterpretRoll[0], &reinterpretRoll[1], &reinterpretRoll[2] );

	QAngle angCur, angNext;

	int iLookup[3][2] = {
		{ 1, 2 },
		{ 2, 1 },
		{ 0, 2 },
	};

	for ( int iDir = 0; iDir < 3; iDir++ )
	{
		colHigh = colors[ iDir ];

		if ( iDir == GetCurrentSelectedAxis() - EDITORAXIS_FIRST )
			colHigh = SELECTED_AXIS_COLOR;

		colLow = colHigh * flColorLowScale;

		IMesh *pMesh = pRenderContext->GetDynamicMesh( true, NULL, NULL, m_matHelper );

		CMeshBuilder mB;
		mB.Begin( pMesh, MATERIAL_QUADS, 2 * iSubDiv );

		VectorAngles( reinterpretRoll[ iLookup[iDir][0] ],
			reinterpretRoll[ iLookup[iDir][1] ],
			angCur );
		angNext = angCur;

		float flCurrentRadius = flRadius - iDir * 0.275f;

		for ( int iStep = 0; iStep < iSubDiv; iStep++ )
		{
			angNext.z += flRotationStep;

			Vector cUp, nUp, cFwd;

			AngleVectors( angCur, &cFwd, NULL, &cUp );
			AngleVectors( angNext, NULL, NULL, &nUp );

			tmp[0] = center
				+ cUp * flCurrentRadius
				+ cFwd * flThickness;

			tmp[1] = center
				+ nUp * flCurrentRadius
				+ cFwd * flThickness;

			tmp[2] = tmp[1] - cFwd * flThickness * 2;
			tmp[3] = tmp[0] - cFwd * flThickness * 2;

			for ( int iPoint = 0; iPoint < 4; iPoint++ )
			{
				mB.Position3fv( tmp[ iPoint ].Base() );
				mB.Color3fv( colHigh.Base() );
				mB.TexCoord2f( 0, 0, 0 );
				mB.AdvanceVertex();
			}

			for ( int iPoint = 3; iPoint >= 0; iPoint-- )
			{
				mB.Position3fv( tmp[ iPoint ].Base() );
				mB.Color3fv( colLow.Base() );
				mB.TexCoord2f( 0, 0, 0 );
				mB.AdvanceVertex();
			}

			angCur = angNext;
		}

		mB.End();
		pMesh->Draw();
	}
}

Vector CMapaddEditor::GetViewOrigin()
{
	const CViewSetup *setup = view->GetViewSetup();
	return setup->origin;
}

QAngle CMapaddEditor::GetViewAngles()
{
	const CViewSetup *setup = view->GetViewSetup();
	return setup->angles;
}

void CMapaddEditor::UpdateCurrentSelectedAxis( int x, int y )
{
	if ( !IsAnythingSelected() ||
		GetEditorInteractionMode() != EDITORINTERACTION_TRANSLATE &&
		GetEditorInteractionMode() != EDITORINTERACTION_ROTATE )
	{
		SetCurrentSelectedAxis( EDITORAXIS_NONE );
		return;
	}

	Vector picker = GetPickingRay( x, y );
	Vector origin = GetViewOrigin();

	Vector center = GetSelectionCenter();
	QAngle orientation = GetSelectionOrientation();

	Ray_t pickerRay;
	pickerRay.Init( origin, origin + picker * MAX_TRACE_LENGTH );

	EDITOR_SELECTEDAXIS idealAxis = EDITORAXIS_SCREEN;

	switch ( GetEditorInteractionMode() )
	{
	case EDITORINTERACTION_TRANSLATE:
		{
			const float flBoxLength = 24.0f + POINT_BOX_SIZE;
			const float flBoxThick = 4.0f;

			Vector boxes[3][2] = {
				{ Vector( POINT_BOX_SIZE, -flBoxThick, -flBoxThick ),
						Vector( flBoxLength, flBoxThick, flBoxThick ) },
				{ Vector( -flBoxThick, POINT_BOX_SIZE, -flBoxThick ),
						Vector( flBoxThick, flBoxLength, flBoxThick ) },
				{ Vector( -flBoxThick, -flBoxThick, POINT_BOX_SIZE ),
						Vector( flBoxThick, flBoxThick, flBoxLength ) },
			};

			float flLastFrac = MAX_TRACE_LENGTH;

			for ( int i = 0; i < 3; i++ )
			{
				CBaseTrace trace;
				if ( IntersectRayWithBox( pickerRay,
					center + boxes[i][0], center + boxes[i][1],
					0.5f, &trace ) )
				{
					if ( trace.fraction > flLastFrac )
						continue;

					flLastFrac = trace.fraction;
					idealAxis = (EDITOR_SELECTEDAXIS)(EDITORAXIS_FIRST + i);
				}
			}

			if ( idealAxis == EDITORAXIS_SCREEN )
			{
				const float flPlaneSize = POINT_BOX_SIZE + 6.0f;

				Vector planes[3][2] = {
					{ Vector( 0, 0, -1 ), Vector( flPlaneSize, flPlaneSize, 1 ) },
					{ Vector( 0, -1, 0 ), Vector( flPlaneSize, 1, flPlaneSize ) },
					{ Vector( -1, 0, 0 ), Vector( 1, flPlaneSize, flPlaneSize ) },
				};

				for ( int i = 0; i < 3; i++ )
				{
					CBaseTrace trace;
					if ( IntersectRayWithBox( pickerRay,
						center + planes[i][0], center + planes[i][1],
						0.5f, &trace ) )
					{
						if ( trace.fraction > flLastFrac )
							continue;

						flLastFrac = trace.fraction;
						idealAxis = (EDITOR_SELECTEDAXIS)(EDITORAXIS_FIRST_PLANE + i);
					}
				}
			}
		}
		break;
	case EDITORINTERACTION_ROTATE:
		{
			const int iSubDiv = 32.0f;
			const float flRadius = 32.0f;
			const float flMaxDist = 4.0f * 4.0f;
			const float flRotationStep = 360.0f / iSubDiv;

			float flBestViewDist = MAX_TRACE_LENGTH;

			Vector reinterpretRoll[3];

			AngleVectors( orientation,
				&reinterpretRoll[0], &reinterpretRoll[1], &reinterpretRoll[2] );

			QAngle angCur, angNext;
			int iLookup[3][2] = {
				{ 1, 2 },
				{ 2, 1 },
				{ 0, 2 },
			};

			for ( int iDir = 0; iDir < 3; iDir++ )
			{
				float flBestDist = MAX_TRACE_LENGTH;

				VectorAngles( reinterpretRoll[ iLookup[iDir][0] ],
					reinterpretRoll[ iLookup[iDir][1] ],
					angCur );
				angNext = angCur;

				for ( int iStep = 0; iStep < iSubDiv; iStep++ )
				{
					angNext.z += flRotationStep;

					Vector cUp, nUp;

					AngleVectors( angCur, NULL, NULL, &cUp );
					AngleVectors( angNext, NULL, NULL, &nUp );

					Ray_t ringRay;
					ringRay.Init( center + cUp * flRadius,
						center + nUp * flRadius );

					float t = 0, s = 0;
					IntersectRayWithRay( pickerRay, ringRay, t, s );

					Vector a = pickerRay.m_Start + pickerRay.m_Delta.Normalized() * t;
					Vector b = ringRay.m_Start + ringRay.m_Delta.Normalized() * s;

					float flDist = ( a - b ).LengthSqr();
					float flStepLengthSqr = ringRay.m_Delta.LengthSqr();

					if ( flDist < flBestDist &&
						flDist < flMaxDist && 
						s * s < flStepLengthSqr &&
						t > 0 && t < (flBestViewDist-7) )
					{
						flBestDist = flDist;
						flBestViewDist = t;

						idealAxis = (EDITOR_SELECTEDAXIS)(EDITORAXIS_FIRST + iDir);
					}

					angCur = angNext;
				}
			}
		}
		break;
	}

	SetCurrentSelectedAxis( idealAxis );
}

void CMapaddEditor::SetEditorActive( bool bActive, bool bView, bool bLights )
{
	if ( bActive && !IsLightingEditorAllowed() )
		return;

	if ( bView )
		m_bActive = bActive;

	if ( bLights )
	{
		m_bLightsActive = bActive;
	}
}

bool CMapaddEditor::IsEditorActive()
{
	return m_bActive;
}

bool CMapaddEditor::IsEditorLightingActive()
{
	return m_bLightsActive;
}

void CMapaddEditor::SetEditorInteractionMode( EDITORINTERACTION_MODE mode )
{
	m_iInteractionMode = mode;
}

CMapaddEditor::EDITORINTERACTION_MODE CMapaddEditor::GetEditorInteractionMode()
{
	return m_iInteractionMode;
}

void CMapaddEditor::FlushEditorLights()
{
	m_hSelectedEnts.Purge();
	m_hEditorEnts.PurgeAndDeleteElements();
}

KeyValues *CMapaddEditor::VmfToKeyValues( const char *pszVmf )
{
	CUtlBuffer vmfBuffer;
	vmfBuffer.SetBufferType( true, true );

	vmfBuffer.SeekPut( CUtlBuffer::SEEK_HEAD, 0 );
	vmfBuffer.PutString( "\"MapaddScript\"\r\n{" );

	KeyValues *pszKV = NULL;

	if ( g_pFullFileSystem->ReadFile( pszVmf, NULL, vmfBuffer ) )
	{
		vmfBuffer.SeekPut( CUtlBuffer::SEEK_TAIL, 0 );
		vmfBuffer.PutString( "\r\n}" );
		vmfBuffer.PutChar( '\0' );

		pszKV = new KeyValues("MapaddScript");

		if ( !pszKV->LoadFromBuffer( "", vmfBuffer ) )
		{
			pszKV->deleteThis();
			pszKV = NULL;
		}
	}

	return pszKV;
}

void CMapaddEditor::KeyValuesToVmf( KeyValues *pKV, CUtlBuffer &vmf )
{
	for ( KeyValues *pKey = m_pKVMA->GetFirstTrueSubKey(); pKey; pKey = pKey->GetNextTrueSubKey() )
	{
		pKey->RecursiveSaveToFile( vmf, 0 );
	}
}

void CMapaddEditor::LoadVmf( const char *pszVmf )
{
	Q_snprintf( m_szCurrentMapadd, sizeof( m_szCurrentMapadd ), "%s", pszVmf );

	KeyValues *pKV = VmfToKeyValues( pszVmf );

	if ( !pKV )
	{
		*m_szCurrentMapadd = 0;
		return;
	}

	ParseMapaddFile( pKV );
	pKV->deleteThis();
}

void CMapaddEditor::SaveCurrentVmf()
{
	Assert( Q_strlen( m_szCurrentMapadd ) > 0 );

	if ( !g_pFullFileSystem->FileExists( m_szCurrentMapadd ) )
	{
		AssertMsg( 0, "Expected file unavailable (moved, deleted)." );
		return;
	}

	ApplyEntitiesToCurrentMapaddFile();

	CUtlBuffer buffer;
	KeyValuesToVmf( m_pKVMA, buffer );

	g_pFullFileSystem->WriteFile( m_szCurrentMapadd, NULL, buffer );
}

const char *CMapaddEditor::GetCurrentVmfPath()
{
	return m_szCurrentMapadd;
}

void CMapaddEditor::ParseMapaddFile( KeyValues *pKeyValues )
{
	Assert( pKeyValues );

	FlushEditorLights();

	if ( m_pKVMA )
		m_pKVMA->deleteThis();

	m_pKVMA = pKeyValues->MakeCopy();

	for ( KeyValues *pKey = m_pKVMA->GetFirstTrueSubKey(); pKey; pKey = pKey->GetNextTrueSubKey() )
	{
		Msg("%s\n", pKey->GetName());

		// why
		char name[64];
		Q_strcpy(name, pKey->GetName());

		if ( Q_strcmp(Q_strlower(name), "entities"))
			continue;
		
		for (KeyValues* pEntKey = pKey->GetFirstTrueSubKey(); pEntKey; pEntKey = pEntKey->GetNextTrueSubKey())
		{
			mapadd_editor_t* pLight = new mapadd_editor_t();

			if (!pLight)
				continue;

			const char * posString = pEntKey->GetString("origin", "0 0 0");
			UTIL_StringToVector(pLight->pos.Base(), posString);

			const char* angString = pEntKey->GetString("angle", "0 0 0");
			UTIL_StringToVector(pLight->ang.Base(), angString);

			Q_strcpy(pLight->name, pEntKey->GetName());

			pLight->hull = GetHullIDFromName(pLight->name);

			if (pEntKey->FindKey("keyvalues"))
				pLight->pKV = pEntKey->FindKey("keyvalues")->MakeCopy();

			AddEditorLight(pLight);
		}
	}
}

void CMapaddEditor::ApplyEntitiesToCurrentMapaddFile()
{
	if(m_pKVMA->FindKey("entities"))
		m_pKVMA->RemoveSubKey(m_pKVMA->FindKey("entities"));

	KeyValues* pKV = m_pKVMA->CreateNewKey();
	pKV->SetName("entities");

	FOR_EACH_VEC( m_hEditorEnts, iLight )
	{
		mapadd_editor_t *pLight = assert_cast< mapadd_editor_t* >( m_hEditorEnts[ iLight ] );

		KeyValues *pKVNew = GetKVFromLight( pLight );
		pKVNew->SetName(pLight->name);
		pKVNew->RemoveSubKey(pKVNew->FindKey("classname"));

		if(pLight->pKV)
			pKVNew->AddSubKey(pLight->pKV);

		pKV->AddSubKey( pKVNew );
	}
}

void CMapaddEditor::GetEditorView( Vector *origin, QAngle *angles )
{
	if ( origin != NULL )
		*origin = m_vecEditorView_Origin;

	if ( angles != NULL )
		*angles = m_angEditorView_Angles;
}

void CMapaddEditor::SetEditorView( const Vector *origin, const QAngle *angles )
{
	if ( origin != NULL )
		m_vecEditorView_Origin = *origin;

	if ( angles != NULL )
		m_angEditorView_Angles = *angles;
}

Vector &CMapaddEditor::GetMoveDirForModify()
{
	return m_vecMoveDir;
}

void CMapaddEditor::AbortEditorMovement( bool bStompVelocity )
{
	m_vecMoveDir.Init();

	if ( bStompVelocity )
		m_vecEditorView_Velocity.Init();
}

void CMapaddEditor::AddEntityFromScreen( int x, int y, KeyValues *pData )
{
	Vector picker = GetPickingRay( x, y );

	Vector origin = GetViewOrigin();
	trace_t tr;
	Ray_t ray;
	Vector mins, maxs;
	GetEntityHullFromID(HULL_HUMAN, mins, maxs);
	ray.Init( origin, origin + picker * MAX_TRACE_LENGTH, mins, maxs);

	UTIL_TraceRay( ray, MASK_SOLID, C_BasePlayer::GetLocalPlayer(), COLLISION_GROUP_NONE, &tr );

	if ( !tr.DidHit() )
		return;

	Vector lightPos = tr.endpos;

	mapadd_ent_t *pLight = new mapadd_editor_t();

	pLight->pos = lightPos;
	pLight->ang = vec3_angle;
	Q_strcpy(pLight->name, "npc_metropolice");
	pLight->hull = HULL_HUMAN;

	AddEditorLight( pLight );

	ClearSelection();
	AddEntityToSelection(pLight);
}

void CMapaddEditor::CopyEntityFromScreen(int x, int y)
{
	Vector picker = GetPickingRay(x, y);

	Vector origin = GetViewOrigin();
	trace_t tr;
	Ray_t ray;
	Vector mins, maxs;
	GetEntityHullFromID(HULL_HUMAN, mins, maxs);
	ray.Init(origin, origin + picker * MAX_TRACE_LENGTH);

	UTIL_TraceRay(ray, MASK_SOLID, C_BasePlayer::GetLocalPlayer(), COLLISION_GROUP_NONE, &tr);

	if (!tr.DidHit())
		return;

	if (tr.DidHitWorld())
		return;

	Vector lightPos = tr.m_pEnt->GetAbsOrigin();

	mapadd_ent_t* pLight = new mapadd_editor_t();

	pLight->pos = lightPos;
	pLight->ang = tr.m_pEnt->GetAbsAngles();
	Q_strcpy(pLight->name, tr.m_pEnt->GetClassname());
	pLight->hull = GetHullIDFromName(pLight->name);

	AddEditorLight(pLight);

	ClearSelection();
	AddEntityToSelection(pLight);
}

bool CMapaddEditor::SelectEntity( int x, int y, bool bGroup, bool bToggle )
{
	Vector picker = GetPickingRay( x, y );

	Vector origin = GetViewOrigin();

	trace_t tr;
	CTraceFilterWorldOnly filter;

	UTIL_TraceLine( origin, origin + picker * MAX_TRACE_LENGTH,
		MASK_SOLID, &filter, &tr );

	bool bSelectionChanged = false;

	if ( tr.DidHit() )
	{
		bool bFound = false;
		BoxTraceInfo_t boxtr;

		FOR_EACH_VEC( m_hEditorEnts, i )
		{
			mapadd_ent_t *l = m_hEditorEnts[ i ];
			Vector maxs, mins;
			GetEntityHullFromID(l->hull, mins, maxs);

			if ( IntersectRayWithBox( tr.startpos, tr.endpos - tr.startpos,
				mins + l->pos, maxs + l->pos, 0.1f, &boxtr ) )
			{
				if ( !bGroup && !bToggle )
					ClearSelection();

				if ( bToggle && IsEntitySelected( l ) )
					RemoveEntityFromSelection( l );
				else
					AddEntityToSelection( l );

				bFound = true;
				bSelectionChanged = true;
				break;
			}
		}

		if ( !bFound && !bGroup )
		{
			ClearSelection();
			bSelectionChanged = true;
		}
	}

	return bSelectionChanged;
}

bool CMapaddEditor::SelectEntities( int *pi2_BoundsMin, int *pi2_BoundsMax,
	bool bGroup, bool bToggle )
{
	if ( !bGroup && !bToggle )
		ClearSelection();

	bool bSelectedAny = false;

	trace_t tr;
	CTraceFilterWorldOnly filter;

	FOR_EACH_VEC( m_hEditorEnts, i )
	{
		mapadd_ent_t *pLight = m_hEditorEnts[ i ];

		const bool bSelected = IsEntitySelected( pLight );

		if ( !bToggle && bSelected )
			continue;

		Vector pos = pLight->pos;
		int iscreenPos[2];

		if ( !GetVectorInScreenSpace( pos, iscreenPos[0], iscreenPos[1] ) )
			continue;

		if ( iscreenPos[ 0 ] < pi2_BoundsMin[ 0 ] ||
			iscreenPos[ 1 ] < pi2_BoundsMin[ 1 ] )
			continue;

		if ( iscreenPos[ 0 ] > pi2_BoundsMax[ 0 ] ||
			iscreenPos[ 1 ] > pi2_BoundsMax[ 1 ] )
			continue;

		UTIL_TraceLine( m_vecEditorView_Origin, pos, MASK_SOLID, &filter, &tr );
		if ( tr.fraction != 1.0f )
			continue;

		if ( bSelected )
			RemoveEntityFromSelection( pLight );
		else
			AddEntityToSelection( pLight );

		bSelectedAny = true;
	}

	return bSelectedAny;
}

KeyValues *CMapaddEditor::GetKVFromLight( mapadd_ent_t *pLight )
{
	mapadd_editor_t *pEditorLight = assert_cast<mapadd_editor_t*>(pLight);

	KeyValues* pKV = pEditorLight->AllocateAsKeyValues();
	Assert( pKV != NULL );

	return pKV;
}

void CMapaddEditor::GetKVFromAll( CUtlVector< KeyValues* > &listOut )
{
	FOR_EACH_VEC( m_hEditorEnts, i )
	{
		listOut.AddToTail( GetKVFromLight( m_hEditorEnts[i] ) );
	}
}

void CMapaddEditor::GetKVFromSelection( CUtlVector< KeyValues* > &listOut )
{
	FOR_EACH_VEC( m_hSelectedEnts, i )
	{
		listOut.AddToTail( GetKVFromLight( m_hSelectedEnts[ i ] ) );
	}
}

void CMapaddEditor::ApplyKVToSelection( KeyValues *pKVChanges )
{
	FOR_EACH_VEC( m_hSelectedEnts, i )
	{
		m_hSelectedEnts[ i ]->ApplyKeyValueProperties( pKVChanges );
	}
	
}

Vector CMapaddEditor::GetPickingRay( int x, int y )
{
	Vector ray_out;

	const CViewSetup *setup = view->GetViewSetup();
	float ratio = (4.0f/3.0f/(setup->width/(float)setup->height));
	float flFov = ScaleFOVByWidthRatio( setup->fov, ratio );

	ScreenToWorld( x, y, flFov, setup->origin, setup->angles, ray_out );

	return ray_out;
}

void CMapaddEditor::AddEditorLight( mapadd_ent_t *pDef )
{
	m_hEditorEnts.AddToTail( pDef );
}

void CMapaddEditor::ClearSelection()
{
	m_hSelectedEnts.Purge();
}

void CMapaddEditor::AddEntityToSelection( mapadd_ent_t *l )
{
	if ( !IsEntitySelected( l ) )
		m_hSelectedEnts.AddToTail( l );
}

void CMapaddEditor::RemoveEntityFromSelection( mapadd_ent_t *l )
{
	m_hSelectedEnts.FindAndRemove( l );
}

bool CMapaddEditor::IsEntitySelected( mapadd_ent_t *l )
{
	return m_hSelectedEnts.HasElement( l );
}

void CMapaddEditor::DeleteSelectedLights()
{
	FOR_EACH_VEC( m_hSelectedEnts, i )
	{
		m_hEditorEnts.FindAndRemove( m_hSelectedEnts[i] );
	}

	m_hSelectedEnts.PurgeAndDeleteElements();
}

int CMapaddEditor::GetNumSelected()
{
	return m_hSelectedEnts.Count();
}

bool CMapaddEditor::IsAnythingSelected()
{
	return GetNumSelected() > 0;
}

Vector CMapaddEditor::GetSelectionCenter()
{
	if ( !IsAnythingSelected() )
	{
		Assert( 0 );
		return vec3_origin;
	}

	if ( m_bSelectionCenterLocked )
		return m_vecSelectionCenterCache;

	CUtlVector< Vector > positions;

	FOR_EACH_VEC( m_hSelectedEnts, i )
	{
		positions.AddToTail( m_hSelectedEnts[ i ]->pos );
	}

#if 0 // median
	Vector min, max;

	CalcBoundaries( positions.Base(), positions.Count(), min, max );

	return min + ( max - min ) * 0.5f;
#else // arth mean
	Vector center = vec3_origin;

	FOR_EACH_VEC( positions, i )
		center += positions[ i ];

	return center / positions.Count();
#endif
}

QAngle CMapaddEditor::GetSelectionOrientation()
{
	if ( !IsAnythingSelected() )
	{
		Assert( 0 );
		return vec3_angle;
	}

	if ( GetNumSelected() > 1 )
	{
		return vec3_angle;
	}
	else
	{
		Assert( m_hSelectedEnts.Count() == 1 );

		return m_hSelectedEnts[ 0 ]->ang;
	}
}

void CMapaddEditor::MoveSelectedEntities( Vector delta )
{
	FOR_EACH_VEC( m_hSelectedEnts, i )
	{
		mapadd_ent_t *l = m_hSelectedEnts[ i ];

		l->pos += delta;

		//l->MakeDirtyXForms();
	}
}

void CMapaddEditor::RotateSelectedEntities( VMatrix matRotate )
{
	FOR_EACH_VEC( m_hSelectedEnts, i )
	{
		mapadd_ent_t *l = m_hSelectedEnts[i];

		VMatrix matCurrent, matDst;
		matCurrent.SetupMatrixOrgAngles( vec3_origin, l->ang );
		MatrixMultiply( matRotate, matCurrent, matDst );
		MatrixToAngles( matDst, l->ang );

		//l->MakeDirtyXForms();
	}

	if ( GetNumSelected() < 2 )
		return;

	Vector center = GetSelectionCenter();

	FOR_EACH_VEC( m_hSelectedEnts, i )
	{
		mapadd_ent_t *l = m_hSelectedEnts[i];

		Vector delta = l->pos - center;

		VMatrix matCurrent, matDst;
		matCurrent.SetupMatrixOrgAngles( delta, vec3_angle );
		MatrixMultiply( matRotate, matCurrent, matDst );
		
		Vector rotatedDelta = matDst.GetTranslation();

		Vector move = rotatedDelta - delta;
		l->pos += move;
	}
}