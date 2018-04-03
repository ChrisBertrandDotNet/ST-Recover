//---------------------------------------------------------------------------
// Copyright Christophe Bertrand.
// This file is licensed under the Microsoft Reciprocal License (Ms-RL). You can find a copy of this license in the file "Ms-RL License.htm", in the root directory of the distribution archive.
//---------------------------------------------------------------------------



#ifndef Classe_DisquetteH
#define Classe_DisquetteH


//---------------------------------------------------------------------------

#include <Classes.hpp>
#include <windows.h>
#include "fdrawcmd.h" // pour le pilote "fdrawcmd.sys".
#include <vector>
#include "Classe_Piste.h"





	#define CouleurSecteurNonLu (0xffffff)
	#define CouleurSecteurLuOK (0xff00)
	#define CouleurSecteurErrone (0xff)
	#define CouleurSecteurDifficile (0xff0000)








	struct secteurs
	{
		bool	lu[85][12];
		bool	difficulte_a_lire[85][12];
		bool	erreur[85][12];
	};


	#pragma pack(push,1)
	struct FD_TIMED_SCAN_RESULT_32 // basé sur FD_TIMED_SCAN_RESULT
	{
			BYTE count;                         // count of returned headers
			BYTE firstseen;                     // offset of first sector detected
			DWORD tracktime;                    // total time for track (in microseconds)
			FD_TIMED_ID_HEADER Headers[32];
	};
	#pragma pack(pop)
	#if ((sizeof(FD_TIMED_SCAN_RESULT)+sizeof(FD_TIMED_ID_HEADER)*32) != (sizeof(FD_TIMED_SCAN_RESULT_32)))
		#error CHRIS : FD_TIMED_SCAN_RESULT a dû être modifié dans fdrawcmd.h.
	#endif



class Classe_Disquette // ------------------------------------------
{
private:	// Déclarations de l'utilisateur

	class Classe_Piste*	classe_Piste_actuelle;


	bool	init_classe_piste_actuelle(unsigned piste,unsigned face);

public:		// Déclarations de l'utilisateur

	secteurs SecteursFaceA;
	secteurs SecteursFaceB;

	struct _infos_en_direct {
		unsigned	Piste_Selectionnee; // La tête est actuellement placée ici.
		unsigned	Face_Selectionnee; // Cette face est actuellement selectionnée.
		unsigned	Secteur_en_traitement_base0; // en base 0
	} infos_en_direct;



	bool	ArchitectureDisqueConnue;
	int	NbSecteursParPiste;
	int NbPistes;
	int NbFaces;
	unsigned NbOctetsParSecteur;

	unsigned 	LecteurSelectionne;  // lecteur: 0=A , 1=B , etc..





	HANDLE hDevice;
	// =====

	bool	Win9X;
	bool	fdrawcmd_sys_installe;

	#pragma pack(push) 
	#pragma pack (1) // Alignement à l'octet près: indispensable ici.
	struct t_secteur_boot_Atari_ST
	{
		unsigned __int16	branchement; // 0
		unsigned char	Texte_Loader[6]; // 2
		unsigned __int8	numero_de_serie_0; // 8
		unsigned __int8	numero_de_serie_1; // 9
		unsigned __int8	numero_de_serie_2; // 10
		unsigned __int16	Octets_par_secteur; // 11
		unsigned __int8	Secteurs_par_cluster; // 13
		unsigned __int16	Nb_secteurs_reserves_boot; // 14
		unsigned __int8	Nb_de_FATs; // 16
		unsigned __int16	Nb_maxi_entrees_dans_racine; // 17
		unsigned __int16	Nb_secteurs_dans_disquette; // 19
		unsigned __int8	Code_Media_Descriptor; // 21 (voir "Bible ST" page 212).
		unsigned __int16	Nb_secteurs_par_FAT; // 22
		unsigned __int16	Nb_secteurs_par_piste; // 24
		unsigned __int16	Nb_de_faces; // 26 ( = nb de têtes).
		unsigned __int16	Nb_secteurs_caches; // 28
		unsigned __int8	contenu[510-30]; // 30
		unsigned __int16	Checksum; // 510 (ST bootable: $1234, soit 0x3412 ?).
	} Secteur_Boot_Atari_ST;   // Doit faire 512 octets, à cause des "sizeof".
	BYTE   _suite_eventuelle_du_secteur_de_demarrage[4096-512]; // coller après "Secteur_Boot_Atari_ST".
	#pragma pack(pop)



	__fastcall Classe_Disquette();
	__fastcall ~Classe_Disquette();


	bool		OuvreDisquette(unsigned lecteur, // lecteur: 0=A , 1=B , etc..
		DWORD temps_alloue_ms, TStrings* LOG_strings,
		volatile bool*	p_annulateur, // si cette variable devient vraie, on annule l'opération en cours.
		bool sauve_infos_pistes_brutes);


	bool	CD_LitSecteur(
		unsigned piste, // tous les arguments sont en base 0.
		unsigned face,
		unsigned secteur_base0,
		DWORD temps_alloue_ms,
		TStrings* LOG_strings,
		struct s_Secteur** pp_secteur, //Pour recevoir un "s_Secteur*".
		volatile bool*	p_annulateur, // si cette variable devient vraie, on annule l'opération en cours.
		bool sauve_infos_pistes_brutes);


	bool		FermeDisquette(void);

	struct infopiste*	CD_Analyse_Temps_Secteurs(
		unsigned piste,
		unsigned face);//,    // tous les arguments sont en base 0.

}; // ==============================================


 // ==============================================


#endif
