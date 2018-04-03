//---------------------------------------------------------------------------
// Copyright Christophe Bertrand.
// This file is licensed under the Microsoft Reciprocal License (Ms-RL). You can find a copy of this license in the file "Ms-RL License.htm", in the root directory of the distribution archive.
//---------------------------------------------------------------------------


#include <vcl.h>
#pragma hdrstop


#include "Unit1.h"
#include "Acces_disque.h"
#include "Classe_Disquette.h"
#include "Analyse_disque.h"
#include "Constantes.h"

#include <strsafe.h>

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;

// ======================



Classe_Disquette* cldisq=NULL;


Classe_Disquette* cldisq_analyse=NULL;
Analyse_Disque* thread_Analyse_Disque=NULL;

const COLORREF coul_piste_paire=0xff8000;
const COLORREF coul_piste_impaire=0x00c040;


// ======================


//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
	PleaseCancelCurrentOperation=false;
	ImageFace0->Picture->Bitmap->Canvas->Brush->Color=Form1->Color;
	ImageFace0->Picture->Bitmap->SetSize(ImageFace0->ClientWidth,ImageFace0->ClientHeight);

	ImageFace1->Picture->Bitmap->Canvas->Brush->Color=Form1->Color;
	ImageFace1->Picture->Bitmap->SetSize(ImageFace1->ClientWidth,ImageFace1->ClientHeight);

	PageControlResultats->ActivePage=TabSheetTablesFaces;

	Application->HintPause=0;
	Application->HintShortPause=0;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ButtonReadDiskClick(TObject *Sender)
{
	// Procédure pour lire les secteurs de la disquette.


	this->MemoLOG->Lines->Clear();

	// Maintenant on lit le disque.
	cldisq=new Classe_Disquette();
	if (cldisq==NULL)
		return;

	if ( cldisq->OuvreDisquette(ComboBoxDisque->ItemIndex,
		Temps_ComboBoxTempsMaxi_en_ms[ComboBoxTempsMaxi->ItemIndex],
		this->MemoLOG->Lines, &PleaseCancelCurrentOperation,
		Form1->CheckBoxSauveInfosPistesBrutes->Checked ))
	{
		// on demande de choisir le fichier .ST à enregistrer.
		if (SaveDialogImageDisque->Execute(Handle))
		{
			const DWORD instant_depart=GetTickCount();
			const DWORD duree_autorisee=Temps_ComboBoxTempsMaxi_en_ms[ComboBoxTempsMaxi->ItemIndex];

			BitBtnAnnule->Enabled=true;
			Acces_Disque* th=new Acces_Disque(true);
			if (th != NULL) {
				th->classe_disque=cldisq;

				// Efface les graphiques (méthode à revoir par la suite).
				this->DrawGridSecteursFaceA->Invalidate();
				this->DrawGridSecteursFaceB->Invalidate();
				PageControlResultats->ActivePage=TabSheetTablesFaces;

				unsigned piste,face,secteur_base0;
				piste=cldisq->infos_en_direct.Piste_Selectionnee;
				face=cldisq->infos_en_direct.Face_Selectionnee;
				secteur_base0=cldisq->infos_en_direct.Secteur_en_traitement_base0;

				// Lance le Thread qui effectue la lecture et qui ordonne le dessin.
				#ifndef _DEBUG
					th->Priority=tpHigher;
				#endif
				th->Resume();

				Sleep(200);
				while(th->Thread_en_route)
				{
					const DWORD temps_ecoule= GetTickCount() - instant_depart;
					const bool temps_depasse= temps_ecoule > duree_autorisee;
					if (PleaseCancelCurrentOperation || temps_depasse) {
						th->Terminate();
						if (PleaseCancelCurrentOperation)
							MemoLOG->Lines->Add("Operation canceled by user (you !).");
						if (temps_depasse)
							MemoLOG->Lines->Add("Time is up, process gaveup (see Giveup time option).");
						break;
					}
					Application->ProcessMessages();
					if (piste!=cldisq->infos_en_direct.Piste_Selectionnee
					|| face!=cldisq->infos_en_direct.Face_Selectionnee
					|| secteur_base0!=cldisq->infos_en_direct.Secteur_en_traitement_base0) {
						piste=cldisq->infos_en_direct.Piste_Selectionnee;
						face=cldisq->infos_en_direct.Face_Selectionnee;
						secteur_base0=cldisq->infos_en_direct.Secteur_en_traitement_base0;
						static char texteinfos[256];
						StringCbPrintf(texteinfos,sizeof(texteinfos)-1,
							"Track:%d Side/Head:%d Sector:%d",
							piste,face,secteur_base0+1);
						LabelInformation->Caption=texteinfos;
					}
					static DWORD ancienne_seconde=temps_ecoule/1000;
					if ((temps_ecoule/1000) != ancienne_seconde)
					{
						static char textetemps[256];
						DWORD d=temps_ecoule/1000;
						const DWORD heure=d/3600;
						d %= 3600;
						const DWORD minute=d/60;
						d %= 60;
						const DWORD seconde=d;
						StringCbPrintf(textetemps,sizeof(textetemps)-1,
							"Time: %u:%02u:%02u",
							heure,minute,seconde);
						LabelTempsEcoule->Caption=textetemps;
						ancienne_seconde = temps_ecoule/1000;
					}
					Sleep(200);
				}

				/* Le passage suivant est nécessaire pour le cas où la fenêtre ait
				été cachée à la fin de la lecture. Sinon, les graphiques apparaitront incomplets
				lorsqu'on agrandira de nouveau la fenêtre. */
				for (int p=0;p<cldisq->NbPistes;p++)
					for (int s=0;s<cldisq->NbSecteursParPiste;s++)
					{
						TGridDrawState ds;
						TRect rect_invalide = DrawGridSecteursFaceA->CellRect(p, s);
						DrawGridSecteursFaceADrawCell(DrawGridSecteursFaceA,p,s,rect_invalide,ds);
						rect_invalide = DrawGridSecteursFaceB->CellRect(p, s);
						DrawGridSecteursFaceADrawCell(DrawGridSecteursFaceB,p,s,rect_invalide,ds);
					}

				Application->ProcessMessages();
				delete th;
				PleaseCancelCurrentOperation=false;
			}
			// fin
			BitBtnAnnule->Enabled=false;
			cldisq->FermeDisquette();
		}
	}
	else
		Application->MessageBox("No disk in drive",Caption.c_str(),MB_OK | MB_ICONERROR);

	MemoLOG->Lines->Add("Operation terminated.");

	delete cldisq;
	cldisq=NULL;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::DrawGridSecteursFaceADrawCell(TObject *Sender, int ACol,
			int ARow, TRect &Rect, TGridDrawState State)
{

	const TDrawGrid* grid=(TDrawGrid*) Sender;

	static int tabicouleurs[2][NB_MAX_PISTES][NB_MAX_SECTEURS_PAR_PISTE]; // Pour mémoriser les couleurs.
	enum icouleurs {
		I_CouleurSecteurNonLu=0,
		I_CouleurSecteurLuOK,
		I_CouleurSecteurDifficile,
		I_CouleurSecteurErrone };
	const COLORREF couleurs[4]={
		CouleurSecteurNonLu,
		CouleurSecteurLuOK,
		CouleurSecteurDifficile,
		CouleurSecteurErrone };

	const unsigned igrid=
		Sender==DrawGridSecteursFaceA ? 0 : 1;

	COLORREF coul=0;

	if (cldisq != NULL)
	{    // on utilise les infos fraiches de la disquette.
		const secteurs* sects=
			Sender==DrawGridSecteursFaceA ? &cldisq->SecteursFaceA : &cldisq->SecteursFaceB;
		unsigned icoul=I_CouleurSecteurNonLu;

		if (sects->lu[ACol][ARow] )
		{
			if (sects->erreur[ACol][ARow] )
			{
				if (sects->difficulte_a_lire[ACol][ARow] )
				{
					icoul=I_CouleurSecteurDifficile;
				}
				else
					icoul=I_CouleurSecteurLuOK;
			}
			else
			{
				icoul=I_CouleurSecteurErrone;
			}
		}
		coul=couleurs[icoul];
		tabicouleurs[igrid][ACol][ARow]=icoul;
	}
	else  // sinon, on utilise la mémoire de la dernière analyse.
	{
		coul=couleurs[tabicouleurs[igrid][ACol][ARow]];
	}

	grid->Canvas->Brush->Color=(TColor) coul;
	grid->Canvas->FillRect(Rect);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ButtonAnalyseClick(TObject *Sender)
{
	this->MemoLOG->Lines->Clear();

	PageControlResultats->ActivePage=TabSheetFaces;
	AnalyseDisquette();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::BitBtnAnnuleClick(TObject *Sender)
{
	const int r=Application->MessageBox(
		"Do you really want to cancel the current operation ?",
		Caption.c_str(),
		MB_YESNO | MB_ICONQUESTION);
	PleaseCancelCurrentOperation=(r == IDYES);
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void __fastcall TForm1::On_recover_maj_piste_FormAnalyse(TMessage &Message)
{
	// Pour recevoir un message personnalisé: WM_recover_maj_piste_FormAnalyse.
	const unsigned piste=Message.WParam;
	const unsigned face=Message.LParam;

	// On calcule des arcs de cercle, en utilisant un temps de 200 ms par piste.

	const DWORD duree_secteur_en_microsecondes=(200000 * 512 / 6250);
	const double PI2 = 3.1415926535897932385 * 2;

	if (thread_Analyse_Disque!=NULL) {
		const FD_TIMED_SCAN_RESULT_32* ap=&thread_Analyse_Disque->Tab_analyse_pistes[piste][face];
		TImage* image= (face==0) ? Form1->ImageFace0 : Form1->ImageFace1;
		image->Picture->Bitmap->Canvas->Pen->Width=2;

		const int rayon_exterieur_piste = 400/2-piste*2 ;
		//const int rayon_interieur_piste = rayon_exterieur_piste-1 ;

		image->Picture->Bitmap->Canvas->Pen->Color= (TColor)  //random(0xffffff);
			(((piste & 1)==0) ? coul_piste_paire : coul_piste_impaire);

		for (int s=0;s < ap->count ;s++ ) {
			const double angle_depart_secteur=
				(double)ap->Headers[s].reltime * PI2 / (double)ap->tracktime;
			const double angle_fin_secteur=
				(double)(ap->Headers[s].reltime+duree_secteur_en_microsecondes)
				* PI2 / (double)ap->tracktime;
			const double xdebut=Cos(angle_depart_secteur)*rayon_exterieur_piste;
			const double ydebut=Sin(angle_depart_secteur)*rayon_exterieur_piste;
			const double xfin=Cos(angle_fin_secteur)*rayon_exterieur_piste;
			const double yfin=Sin(angle_fin_secteur)*rayon_exterieur_piste;

			// petites macros pour se placer au centre de l'image, et pas à l'envers.
			#define x(posXrelative) (200+(posXrelative))
			#define y(posYrelative) (200-(posYrelative))

			image->Picture->Bitmap->Canvas->Arc(
				x(-rayon_exterieur_piste), // X1,
				y(-rayon_exterieur_piste),// int Y1,
				x(rayon_exterieur_piste),// int X2,
				y(rayon_exterieur_piste),// int Y2,
				x(xdebut),// int X3,
				y(ydebut),// int Y3,
				x(xfin),//	int X4,
				y(yfin));// int Y4);
			#undef x
			#undef y
		}
		image->Invalidate();
	}


}
//---------------------------------------------------------------------------
bool	__fastcall TForm1::AnalyseDisquette(void)
{
	// Appelé par le bouton Analyse de Form1.

	const DWORD instant_depart=GetTickCount();
	const DWORD duree_autorisee=Temps_ComboBoxTempsMaxi_en_ms[ComboBoxTempsMaxi->ItemIndex];


	cldisq_analyse=new Classe_Disquette();
	if (cldisq_analyse==NULL)
	{
		return false;
	}

	bool OK=false;

	if (cldisq_analyse->OuvreDisquette(ComboBoxDisque->ItemIndex,
		Temps_ComboBoxTempsMaxi_en_ms[ComboBoxTempsMaxi->ItemIndex],
		this->MemoLOG->Lines, &PleaseCancelCurrentOperation,
		Form1->CheckBoxSauveInfosPistesBrutes->Checked) )
	{
		if ( ! cldisq_analyse->fdrawcmd_sys_installe) {
			Application->MessageBox("'fdrawcmd.sys' is needed, and not installed. See your manual for more information.",Application->Name.c_str(),MB_OK | MB_ICONERROR);
			OK=true;
		}
		else
		{
			BitBtnAnnule->Enabled=true;
			{
				ImageFace0->Picture->Bitmap->Canvas->FillRect(
					ImageFace0->Picture->Bitmap->Canvas->ClipRect);
				ImageFace1->Picture->Bitmap->Canvas->FillRect(
					ImageFace1->Picture->Bitmap->Canvas->ClipRect);
				// trace une ligne pour indiquer le marqueur de début de la piste.
				ImageFace0->Picture->Bitmap->Canvas->MoveTo(200,200);
				ImageFace0->Picture->Bitmap->Canvas->LineTo(400,200);
				ImageFace1->Picture->Bitmap->Canvas->MoveTo(200,200);
				ImageFace1->Picture->Bitmap->Canvas->LineTo(400,200);
			}

			thread_Analyse_Disque=new Analyse_Disque(true);
			if (thread_Analyse_Disque != NULL) {
				thread_Analyse_Disque->classe_disque=cldisq_analyse;

				unsigned piste,face,secteur;
				piste=cldisq_analyse->infos_en_direct.Piste_Selectionnee;
				face=cldisq_analyse->infos_en_direct.Face_Selectionnee;

				#ifndef _DEBUG
					thread_Analyse_Disque->Priority=tpHigher;
				#endif
				thread_Analyse_Disque->Resume();

				Sleep(200);
				while(thread_Analyse_Disque->Thread_en_route)
				{
					const DWORD temps_ecoule= GetTickCount() - instant_depart;
					const bool temps_depasse= temps_ecoule > duree_autorisee;
					if (PleaseCancelCurrentOperation || temps_depasse) {
						thread_Analyse_Disque->Terminate();
						if (PleaseCancelCurrentOperation)
							MemoLOG->Lines->Add("Operation canceled by user (you !).");
						if (temps_depasse)
							MemoLOG->Lines->Add("Time is up, process giveup (see Giveup time option).");
						break;
					}
					Application->ProcessMessages();
					if (piste!=cldisq_analyse->infos_en_direct.Piste_Selectionnee
					|| face!=cldisq_analyse->infos_en_direct.Face_Selectionnee
					) { 
						piste=cldisq_analyse->infos_en_direct.Piste_Selectionnee;
						face=cldisq_analyse->infos_en_direct.Face_Selectionnee;
						static char texteinfos[256];
						StringCbPrintf(texteinfos,sizeof(texteinfos)-1,
							"Track:%d Side/Head:%d",
							piste,face);//,secteur+1);
						LabelInformation->Caption=texteinfos;
					}
					Sleep(200);
				}

				Application->ProcessMessages();
				delete thread_Analyse_Disque;
				thread_Analyse_Disque=NULL;
				PleaseCancelCurrentOperation=false;
			}
			BitBtnAnnule->Enabled=false;
		}
		// fin
		cldisq_analyse->FermeDisquette();
	}
	else
	{
		Application->MessageBox("No disk in drive",Caption.c_str(),MB_OK | MB_ICONERROR);
		OK=true;
	}

	delete cldisq_analyse;
	cldisq_analyse=NULL;

	MemoLOG->Lines->Add("Operation terminated.");

	return OK;
}


void __fastcall TForm1::DrawGridSecteursFaceAMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
	AnsiString s;
	TDrawGrid* dg=(TDrawGrid*) Sender;
	TGridCoord gc=dg->MouseCoord (X,Y);
	s.printf("Track %d - Sector %d",
		gc.X, gc.Y+1);
	dg->Hint=s;
}
//---------------------------------------------------------------------------

