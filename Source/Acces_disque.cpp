//---------------------------------------------------------------------------
// Copyright Christophe Bertrand.
// This file is licensed under the Microsoft Reciprocal License (Ms-RL). You can find a copy of this license in the file "Ms-RL License.htm", in the root directory of the distribution archive.
//---------------------------------------------------------------------------


#include <vcl.h>
#pragma hdrstop

#include "Acces_disque.h"
#include "Unit1.h"
//#include "Classe_Disquette.h"
#pragma package(smart_init)



//---------------------------------------------------------------------------

//   Important : les méthodes et les propriétés des objets de la CVL peuvent uniquement
//   être utilisées dans une méthode appelée en utilisant Synchronize, comme suit :
//
//      Synchronize(&UpdateCaption);
//
//   où UpdateCaption serait de la forme :
//
//      void __fastcall Acces_Disque::UpdateCaption()
//      {
//        Form1->Caption = "Mis à jour dans un thread";
//      }
//---------------------------------------------------------------------------

__fastcall Acces_Disque::Acces_Disque(bool CreateSuspended)
	: TThread(CreateSuspended)
{
}
//---------------------------------------------------------------------------
void Acces_Disque::SetName()
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = "Accès disque";
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
void __fastcall Acces_Disque::Execute()
{
	SetName();
	//---- Placer le code du thread ici----

	if (classe_disque==NULL)
	{
		this->ReturnValue=false;
		return;
	}

	// Crée le fichier pour enregistrer l'image du disque:

	const HANDLE himagefile=CreateFile(
		Form1->SaveDialogImageDisque->FileName.c_str(),//LPCTSTR lpFileName,
		GENERIC_WRITE ,//DWORD dwDesiredAccess,
		FILE_SHARE_READ ,//DWORD dwShareMode,
		NULL, //LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		CREATE_ALWAYS,//DWORD dwCreationDisposition,
		FILE_ATTRIBUTE_NORMAL,//DWORD dwFlagsAndAttributes,
		NULL);//HANDLE hTemplateFile	);
	if (himagefile == INVALID_HANDLE_VALUE)
	{
		return; // erreur d'écriture.
	}


	Thread_en_route=true;

	const DWORD heure_maxi=
		Temps_ComboBoxTempsMaxi_en_ms[Form1->ComboBoxTempsMaxi->ItemIndex]
		+ GetTickCount();


	std::vector<BYTE> phrase;
	{
		phrase.reserve(classe_disque->NbOctetsParSecteur);
		const BYTE repetition[]="======== SORRY, THIS SECTOR CANNOT BE READ FROM FLOPPY DISK BY ST RECOVER. ========\r\n";
		for (unsigned iph=0;iph<classe_disque->NbOctetsParSecteur;iph+=sizeof(repetition)-1)
		{
			int tcop=sizeof(repetition)-1;
			if ((iph+tcop) > classe_disque->NbOctetsParSecteur)
				tcop=classe_disque->NbOctetsParSecteur-iph;
			memcpy(&phrase[iph],repetition,tcop);
		}
	}

	s_Secteur*	p_infos_secteur=NULL;




		// Lecture des secteurs

	for (int p=0; p<classe_disque->NbPistes; p++ )
	{
		if (Terminated)
		{
			break;
		}
		for (int f=0; f<classe_disque->NbFaces; f++ )
		{
			if (Terminated)
			{
				break;
			}
			static BYTE contenu_secteurs_piste[6400];
			BYTE*		pcontenu=contenu_secteurs_piste;
			grid=
				f==0 ? Form1->DrawGridSecteursFaceA : Form1->DrawGridSecteursFaceB;
			secteurs* sects=
				f==0 ? &classe_disque->SecteursFaceA : &classe_disque->SecteursFaceB;
			for (int s=0; s<classe_disque->NbSecteursParPiste; s++)
			{
				if (Terminated)
				{
					break;
				}
				const bool OK=classe_disque->CD_LitSecteur(p,f,s,
					heure_maxi-GetTickCount(),Form1->MemoLOG->Lines,
					&p_infos_secteur, &Form1->PleaseCancelCurrentOperation,
					Form1->CheckBoxSauveInfosPistesBrutes->Checked );
				if ( ! Form1->PleaseCancelCurrentOperation)
				{
					sects->lu[p][s]=true;
					sects->difficulte_a_lire[p][s]= // difficulté si on n'a pas pu lire en mode normal (avec le simple controleur).
						p_infos_secteur->Lecture_normale_par_controleur_essayee
						&& ! p_infos_secteur->Lecture_normale_par_controleur_reussie;
					sects->erreur[p][s] = OK;
					if (((pcontenu-contenu_secteurs_piste)+p_infos_secteur->Taille_en_octets)<=(int)sizeof(contenu_secteurs_piste))
					{
						if (OK) // on copie les données du secteur.
							memcpy(pcontenu,p_infos_secteur->pContenu,p_infos_secteur->Taille_en_octets);
						else // si le secteur n'a pas pu être lu, on met une phrase comme contenu, sinon on aurait un décalage dans l'image disque.
						{
								memcpy(pcontenu,&phrase[0],p_infos_secteur->Taille_en_octets);
						}
						pcontenu += p_infos_secteur->Taille_en_octets;// InfosSect->Taille;
					}

					rect_invalide = grid->CellRect(p, s);

					InvalidateRect(grid->Handle, &rect_invalide, TRUE);
				} // endif ( ! Terminated)
			}
			{
				DWORD NumberOfBytesWritten=0;
				/*BOOL ecritureok=*/ WriteFile(
					himagefile,//HANDLE hFile,
					contenu_secteurs_piste,//LPCVOID lpBuffer,
					pcontenu-contenu_secteurs_piste,//DWORD nNumberOfBytesToWrite,
					&NumberOfBytesWritten,//LPDWORD lpNumberOfBytesWritten,
					NULL);//LPOVERLAPPED lpOverlapped
			}
		}
	} // next p

	Application->ProcessMessages();  // laisse l'affichage se mettre à jour.

	phrase.clear();

	CloseHandle(himagefile);

	Thread_en_route=false;
	this->ReturnValue=true;
}
//---------------------------------------------------------------------------
void __fastcall Acces_Disque::MetAJourLAffichage()
{
	InvalidateRect(grid->Handle, &rect_invalide, TRUE);
}
