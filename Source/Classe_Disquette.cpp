//---------------------------------------------------------------------------
// Copyright Christophe Bertrand.
// This file is licensed under the Microsoft Reciprocal License (Ms-RL). You can find a copy of this license in the file "Ms-RL License.htm", in the root directory of the distribution archive.
//---------------------------------------------------------------------------



#pragma hdrstop

#include "Classe_Disquette.h"
#include "Constantes.h"


//---------------------------------------------------------------------------

#pragma package(smart_init)

#define min(a,b) ((a)<=(b)) ? a : b




__fastcall Classe_Disquette::Classe_Disquette(void)
{
	ArchitectureDisqueConnue=false;
	hDevice=INVALID_HANDLE_VALUE;
	Win9X=false;
	fdrawcmd_sys_installe=false;
	classe_Piste_actuelle=NULL;
}
//---------------------------------------------------------------------------
bool		Classe_Disquette::OuvreDisquette(unsigned lecteur, // lecteur: 0=A , 1=B , etc..
	DWORD temps_alloue_ms, TStrings* LOG_strings,
	volatile bool*	p_annulateur, // si cette variable devient vraie, on annule l'opération en cours.
	bool sauve_infos_pistes_brutes)
{
	ArchitectureDisqueConnue=false;
	const DWORD limite_temps=GetTickCount()+temps_alloue_ms;

	LecteurSelectionne=lecteur;

	// Valeurs par défaut:
	NbSecteursParPiste=NB_MAX_SECTEURS_PAR_PISTE;
	NbPistes=NB_MAX_PISTES;
	NbFaces=2;
	NbOctetsParSecteur=512;

	memset(&SecteursFaceA,0,sizeof(SecteursFaceA));
	memset(&SecteursFaceB,0,sizeof(SecteursFaceB));

	// Creating handle to vwin32.vxd (win 9x)
	hDevice = CreateFile ( "\\\\.\\vwin32",
    0,
    0,
    NULL,
    0,
    FILE_FLAG_DELETE_ON_CLOSE,
    NULL );

	Win9X = hDevice != INVALID_HANDLE_VALUE;

	if ( ! Win9X )
	{
				// win NT/2K/XP code
		{
			// Essaie d'accéder au pilote étendu "fdrawcmd.sys".
			// Voir en http://simonowen.com/fdrawcmd/
			char szDev[] = "\\\\.\\fdraw0"; // 0=A:  1=B:
			szDev[9] += lecteur;
			hDevice = CreateFile(szDev, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
			fdrawcmd_sys_installe = ((hDevice != NULL) && (hDevice != INVALID_HANDLE_VALUE));
		}

		if ( ! fdrawcmd_sys_installe)
		{
			// On se contente du pilote normal de Windows, très limité.
			char _devicename[] = "\\\\.\\A:";
			_devicename[4] += lecteur;

			// Creating a handle to disk drive using CreateFile () function ..
			hDevice = CreateFile(_devicename,
								GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
								NULL, OPEN_EXISTING, 0, NULL);
		}
	}

	bool	OK=((hDevice != NULL) && (hDevice != INVALID_HANDLE_VALUE));

	if (OK)
	{
		// On va tester s'il y a réellement un disque dans le lecteur.
		// Sinon on a droit à des mouvements de têtes rapides et dans le vide.
		if ( fdrawcmd_sys_installe)
		{
			// Avec ce pilote (fdrawcmd.sys), on est forcé d'utiliser une autre méthode.
			DWORD dwRet;
			// On a intérêt à remettre à zéro le controleur de disquettes.
			OK = DeviceIoControl(hDevice, IOCTL_FD_RESET, NULL, NULL, NULL, 0, &dwRet, NULL);
			infos_en_direct.Piste_Selectionnee=0; // La tête est actuellement placée ici.
			infos_en_direct.Face_Selectionnee=0; // Cette face est actuellement selectionnée.
      infos_en_direct.Secteur_en_traitement_base0=0; // à priori.

			// Checks whether a disk is present in the drive
			OK &= DeviceIoControl(hDevice, IOCTL_FD_CHECK_DISK, NULL, NULL, NULL, 0, &dwRet, NULL);

			// Recalibrer la piste.
			OK &= DeviceIoControl(hDevice, IOCTL_FDCMD_RECALIBRATE, NULL, 0, NULL, 0, &dwRet, NULL);

			// set data rate to double-density
			BYTE datarate;
			datarate = FD_RATE_250K;
			OK &= DeviceIoControl(hDevice, IOCTL_FD_SET_DATA_RATE, &datarate, sizeof(datarate), NULL, 0, &dwRet, NULL);
		}
	}

	if (OK)
	{
		s_Secteur* p_infos_secteur=NULL;
		if (
			CD_LitSecteur(0, 0, 0,	 // tous les arguments sont en base 0.
				limite_temps-GetTickCount(),LOG_strings, &p_infos_secteur, p_annulateur,
				sauve_infos_pistes_brutes ) )
		{
			Secteur_Boot_Atari_ST.Octets_par_secteur = // j'ai déjà eu une valeur de "2" ici.
				Secteur_Boot_Atari_ST.Octets_par_secteur >= 128
				? Secteur_Boot_Atari_ST.Octets_par_secteur
				: 512;

			if (Secteur_Boot_Atari_ST.Nb_secteurs_par_piste == 0) {
				OK=false; // pb de lecture du secteur de démarrage (ça arrive parfois).
			}

			if (OK)
			{
				NbSecteursParPiste=Secteur_Boot_Atari_ST.Nb_secteurs_par_piste;
				NbPistes=
					Secteur_Boot_Atari_ST.Nb_secteurs_dans_disquette
					/ Secteur_Boot_Atari_ST.Nb_secteurs_par_piste
					/ Secteur_Boot_Atari_ST.Nb_de_faces;
				NbFaces=Secteur_Boot_Atari_ST.Nb_de_faces;
				NbOctetsParSecteur=Secteur_Boot_Atari_ST.Octets_par_secteur;
				ArchitectureDisqueConnue=true;

				if (NbPistes > NB_MAX_PISTES)
					NbPistes = NB_MAX_PISTES; // sécurité
				if (NbSecteursParPiste > NB_MAX_SECTEURS_PAR_PISTE)
					NbSecteursParPiste = NB_MAX_SECTEURS_PAR_PISTE; // sécurité
			}
		}
	}

	if (OK)
		LOG_strings->Add("Disk open correctly");
	else
		LOG_strings->Add("Warning: Disk could NOT be open, or maybe the information in the boot sector are not adequate");

	return OK;
}
// ====================================
bool	Classe_Disquette::CD_LitSecteur(
	unsigned piste, // tous les arguments sont en base 0.
	unsigned face,
	unsigned secteur_base0,
	DWORD temps_alloue_ms,
	TStrings* LOG_strings,
	s_Secteur** pp_secteur, //Pour recevoir un "s_Secteur*".
	volatile bool*	p_annulateur, // si cette variable devient vraie, on annule l'opération en cours.
	bool sauve_infos_pistes_brutes)
{
	if (*p_annulateur)
		return false;

	init_classe_piste_actuelle(piste,face);
	
	if (classe_Piste_actuelle==NULL)
		return false; // erreur.
	// -------------------------------

	infos_en_direct.Secteur_en_traitement_base0=secteur_base0; // vrai qu'on parvienne à le lire ou pas.

	s_Secteur* p_s_secteur=NULL;
	const bool	OK= classe_Piste_actuelle->CP_LitSecteur(
		this,//Classe_Disquette* cl_disquette, // classe appelante.		piste,//unsigned piste, // tous les arguments sont en base 0.
		piste,
		face,//unsigned face,
		secteur_base0,//unsigned secteur_base0,
		temps_alloue_ms,//DWORD temps_alloue_ms,
		&p_s_secteur,//s_Secteur** p_p_s_secteur) // Ecrit un pointeur sur une classe dans la mémoire fournie.
		LOG_strings,//TStrings* LOG_strings);
		p_annulateur, // si cette variable devient vraie, on annule l'opération en cours.
		sauve_infos_pistes_brutes);
	*pp_secteur=p_s_secteur;
	return OK;
}
// ====================================
bool		Classe_Disquette::FermeDisquette(void)
{
	if (classe_Piste_actuelle != NULL)
	{
		delete classe_Piste_actuelle;
		classe_Piste_actuelle=NULL;
	}

	bool OK=false;
	if ((hDevice != NULL) && (hDevice != INVALID_HANDLE_VALUE))
	{
		OK=CloseHandle(hDevice);
		if (OK)
		{
			hDevice=NULL;
		}
	}

	return OK;
}
// ====================================
__fastcall Classe_Disquette::~Classe_Disquette()
{
	Classe_Disquette::FermeDisquette();
}
// ====================================
infopiste*	Classe_Disquette::CD_Analyse_Temps_Secteurs(
	unsigned piste,
	unsigned face)//,    // tous les arguments sont en base 0.
{
	init_classe_piste_actuelle(piste,face);

	if (classe_Piste_actuelle==NULL)
		return NULL; // erreur.
	// -------------------------------

	return	classe_Piste_actuelle->CP_Analyse_Temps_Secteurs(
		this,//class Classe_Disquette* classe_disquette, // classe appelante.
		piste,//unsigned piste,
		face);//unsigned face);//,    // tous les arguments sont en base 0.
}
// ====================================

	// -------------------------------
	// On utilise la Classe_Piste.
bool	Classe_Disquette::init_classe_piste_actuelle(unsigned piste,unsigned face)
{
	{
		bool doitcreerclasse=false;
		if (classe_Piste_actuelle == NULL)
			doitcreerclasse=true;
		else
			if (
				(classe_Piste_actuelle->get_Numero_de_piste_base_0() != (int)piste)
				|| (classe_Piste_actuelle->get_Numero_de_face_base_0() != (int)face) )
			{
				delete classe_Piste_actuelle; // Seul endroit où on détruit cette classe.
				classe_Piste_actuelle=NULL;
				doitcreerclasse=true;
			}
		if (doitcreerclasse)
			classe_Piste_actuelle=new Classe_Piste(piste,face);
	}
	return classe_Piste_actuelle != NULL;
}
	// ====================================
