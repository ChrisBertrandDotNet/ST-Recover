//---------------------------------------------------------------------------
// Copyright Christophe Bertrand.
// This file is licensed under the Microsoft Reciprocal License (Ms-RL). You can find a copy of this license in the file "Ms-RL License.htm", in the root directory of the distribution archive.
//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Analyse_disque.h"
#include "Unit1.h"

#pragma package(smart_init)
//---------------------------------------------------------------------------

//   Important : les méthodes et les propriétés des objets de la VCL peuvent uniquement
//   être utilisées dans une méthode appelée en utilisant Synchronize, comme suit :
//
//      Synchronize(&UpdateCaption);
//
//   où UpdateCaption serait de la forme :
//
//      void __fastcall Analyse_Disque::UpdateCaption()
//      {
//        Form1->Caption = "Mis à jour dans un thread";
//      }
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
__fastcall Analyse_Disque::Analyse_Disque(bool CreateSuspended)
	: TThread(CreateSuspended)
{
}
//---------------------------------------------------------------------------
void Analyse_Disque::SetName()
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = "Analyse du disque par Thread";
	info.dwThreadID = -1;
	info.dwFlags = 0;

	__try
	{
		RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD),(DWORD*)&info );
	}
	__except (EXCEPTION_CONTINUE_EXECUTION)
	{
	}
}
//---------------------------------------------------------------------------
void Analyse_Disque::metajour()
{
	Application->ProcessMessages();  // laisse l'affichage se mettre à jour.
}
//---------------------------------------------------------------------------
void __fastcall Analyse_Disque::Execute()
{
	SetName();
	//---- Placer le code du thread ici----
	if (classe_disque==NULL) {
		this->ReturnValue=false;
		return;
	}
	Thread_en_route=true;
		// --------------

	// Analyse des pistes
	for (int p=0; p<classe_disque->NbPistes; p++ ) {
		if (Terminated) {
			break;
		}
		for (int f=0; f<classe_disque->NbFaces; f++ ) {
			if (Terminated) {
				break;
			}
			infopiste* ip=classe_disque->CD_Analyse_Temps_Secteurs(p,f);//,false,false);
			if (ip->OperationReussie) {
				/* on copie les données de la piste dans le tableau de toute la disquette.
				 Ainsi, l'autre thread peut y accéder en parallèle, même en ayant
				plusieurs messages de retard (on ne sait jamais). */
				Tab_analyse_pistes[p][f]=*ip->fdrawcmd_Timed_Scan_Result;
					// On envoie un message personnalisé: WM_recover_maj_piste_FormAnalyse.
					//const Donnes_recover_maj_piste_FormAnalyse infos={p,f};
					PostMessage(
						Form1->Handle,WM_recover_maj_piste_FormAnalyse,p,f);
			}
		}
	}

	Application->ProcessMessages();  // laisse l'affichage se mettre à jour.

	// fin ----------
	this->ReturnValue=true;
	Thread_en_route=false;
}
//---------------------------------------------------------------------------
