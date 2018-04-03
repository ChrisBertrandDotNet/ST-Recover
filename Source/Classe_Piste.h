//---------------------------------------------------------------------------
// Copyright Christophe Bertrand.
// This file is licensed under the Microsoft Reciprocal License (Ms-RL). You can find a copy of this license in the file "Ms-RL License.htm", in the root directory of the distribution archive.
//---------------------------------------------------------------------------



#ifndef Classe_PisteH
#define Classe_PisteH
//---------------------------------------------------------------------------


#include <vector>
#include <windows.h>

#include "Classe_Disquette.h"

// =================== Des d�finitions g�n�rales ===========================

#define CP_NB_MAXI_SECTEURS_PAR_PISTE_BRUTE (6400/128)


	struct s_Secteur
	{
		//int		Numero_en_base_1;
		bool	Lu_correctement;
		bool	Lecture_normale_par_controleur_essayee; // lecture normale par le controleur.
		bool	Lecture_normale_par_controleur_reussie;
		//bool	Lecture_par_Piste_Brute_essaye; // lecture dans la piste brute.
		bool	Lecture_par_Piste_Brute_reussie; // lecture dans la piste brute.
		int		Nombre_essais_lecture; // 1=Lu du 1er coup. Sinon, �a a �t� plus difficile.
		int		Taille_en_octets;
		//std::vector<BYTE>*	pContenu; // M�moire.
		BYTE*	pContenu; // M�moire.
	};

	// TODO: "infopiste" devrait �tre supprim�, en faveur de la simple Classe_Piste.
	struct infopiste
	{
		bool									OperationReussie;
		//unsigned __int8*			ContenuPiste;
		//unsigned							TaillePiste;
		//FD_SCAN_RESULT*						fdrawcmd_Scan_Result; GARDER pour plus tard.
		struct FD_TIMED_SCAN_RESULT_32*	fdrawcmd_Timed_Scan_Result; // N� secteurs en base 1.
	};
	


// ===================== La classe en elle-m�me =========================

class Classe_Piste // ------------------------------------------
{
public:

	struct _Infos_Secteurs_Dans_Piste_brute
	{
		//bool			Donnees_bien_lues;
		unsigned	Index_Dans_Contenu_Piste_Codee; // Donn�es du secteur, dans la piste source.
		unsigned	Index_Dans_Contenu_Piste_Decodee; // Donn�es du secteur, dans la piste d�cod�e.
		bool			Secteur_identifie;
		bool			ID_trouve_directement;
		
		// Les valeurs suivantes d'ID ne sont valables que si on a trouv� un ID pour ce secteur.
		unsigned	ID_Piste;
		unsigned	ID_Face;
		unsigned	ID_Secteur_base1; // en base 1.
		unsigned	ID_Taille; // 2=512 3=1024, etc.. : soit 2^t*128.
	};


private:	// D�clarations de l'utilisateur

	int			Piste_base0; //	Num�ro de Piste, en base 0.
	int			Face_base0; //	Num�ro de Face, en base 0.

	struct Infos_Secteurs_Piste_brute_16ko
	{
		unsigned	Nb_Secteurs_trouves; // y compris les r�p�titions.
		unsigned __int8		Contenu_Piste_codee[16*1024];
		unsigned __int8		Contenu_Piste_decodee[16*1024];
		_Infos_Secteurs_Dans_Piste_brute	Infos_Secteur[128]; // environ 128 secteurs de 128 octets au maxi dans une piste brute, de 6250 octets lue pls fois, mais sur 16 ko..
	};
	//struct Infos_Secteurs_Piste_brute_16ko CP_Resultat_Piste_Brute;


	// M�moire pour stocker le contenu de tous les secteurs de cette piste:
	struct
	{
		BYTE		memoire[6400];
		//std::vector<int>	index_memoire_secteurs_base1; // si index=-1, alors pas encore cr��.
		int	index_memoire_secteurs_base1[CP_NB_MAXI_SECTEURS_PAR_PISTE_BRUTE]; // si index=-1, alors pas encore cr��.
		unsigned	index_memoire_libre; // 0 au d�but.
	} contenu_secteurs;

	//s_Secteur* reserve_structure_secteur(unsigned secteur_base0);
	BYTE* reserve_memoire_secteur(unsigned secteur_base0, unsigned nombre_octets);

	struct bloc
	{
		int	instant_depart; // index en micro-secondes (� partir du d�but de la piste).
		int duree_espace_libre_en_1er;		// il y a toujours une zone
		int	numero_secteur_base1_en_2eme; // forc�ment occup�.
		int	taille_secteur_en_octets_code_bits; // 0=128 octets, 1=256, 2=512, .. , 7=16384.
		bool	trouve_par_le_controleur; // sinon, il est trouv� par lecture brute de la piste.
	};



	// ------------------ Fonctions priv�es ---------------


	bool	CP_identifie_secteurs_bruts( // Renvoie si "secteur_base0" a �t� trouv�.
		class Classe_Disquette* classe_disquette, // classe appelante.
		unsigned piste, // tous les arguments sont en base 0.
		unsigned face,
		unsigned secteur_base0,
		TStrings* LOG_strings,
		Infos_Secteurs_Piste_brute_16ko &Resultat_Piste_Brute,
		BYTE* p_memoire_secteur); // L� o� on copiera AUSSI les donn�es du secteur recherch�.

	bool	CP_selectionne_piste_et_face( // seulement avec "fdrawcmd.sys".
		Classe_Disquette* classe_disquette, // classe appelante.
		unsigned piste, // en base 0.
		unsigned face); // en base 0.

	bool	Init_et_complete_tableau_blocs_zones(
		Classe_Disquette* classe_disquette, // classe appelante.
		std::vector<struct bloc>*	tableau_blocs,
		infopiste* piste_timer,
		TStrings* LOG_strings,
		int duree_piste_1_tour); // en microsecondes.

	bool DecodeTrack (Infos_Secteurs_Piste_brute_16ko* Resultat);


public:		// D�clarations de l'utilisateur

	// les informations de secteurs sont stock�s dans un tableau � taille fixe.
	struct s_Secteur	Tableau_des_Secteurs_en_base_0[CP_NB_MAXI_SECTEURS_PAR_PISTE_BRUTE]; // L'index de ce tableau est en base 0.

	bool	sauvegarde_infos_pistes_brutes_deja_effectuee;

	// ------------------ Fonctions publiques ---------------

	Classe_Piste(int Numero_de_piste_base_0, int Numero_de_face_base_0);

	~Classe_Piste();

	int	get_Numero_de_piste_base_0(void);
	int	get_Numero_de_face_base_0(void);

	bool	Classe_Piste::CP_LitSecteur(
		class Classe_Disquette* classe_disquette, // classe appelante.
		unsigned piste, // tous les arguments sont en base 0.
		unsigned face,
		unsigned secteur_base0,
		DWORD temps_alloue_ms,
		s_Secteur** p_p_s_secteur, // Ecrit un pointeur sur une classe dans la m�moire fournie.
		TStrings* LOG_strings,
		volatile bool*	p_annulateur, // si cette variable devient vraie, on annule l'op�ration en cours.
		bool sauve_infos_pistes_brutes);

	bool	Classe_Piste::CP_LitSecteur_ANCIEN( // POUR ARCHIVES.
		Classe_Disquette* classe_disquette, // classe appelante.
		unsigned piste, // tous les arguments sont en base 0.
		unsigned face,
		unsigned secteur_base0,
		DWORD temps_alloue_ms,
		s_Secteur** p_p_s_secteur, // Ecrit un pointeur sur une classe dans la m�moire fournie.
		TStrings* LOG_strings,
		volatile bool*	p_annulateur, // si cette variable devient vraie, on annule l'op�ration en cours.
		bool sauve_infos_pistes_brutes);

	infopiste*	CP_Analyse_Temps_Secteurs( // Analyse la piste, et fourni une carte temporelle des secteurs.
		class Classe_Disquette* classe_disquette, // classe appelante.
		unsigned piste,
		unsigned face);//,    // tous les arguments sont en base 0.

	bool	Classe_Piste::CP_analyse_piste_brute(// on lit la piste brute.
		bool Purement_informatif,// Ne modifie pas les donn�es de secteur. Sinon, on en extrait les informations vers les secteurs.
		Classe_Disquette* classe_disquette, // classe appelante.
		DWORD temps_alloue_ms,
		s_Secteur** p_p_s_secteur, // Ecrit un pointeur sur une classe dans la m�moire fournie.
		TStrings* LOG_strings,
		volatile bool*	p_annulateur, // si cette variable devient vraie, on annule l'op�ration en cours.
		bool sauve_infos_pistes_brutes, // Dans un fichier texte et un fichier binaire.
		struct Infos_Secteurs_Piste_brute_16ko* Resultat_Piste_Brute);


private:   // -----------------------------------------

	bool LitSecteursPisteBrute( // Lit la piste actuellement s�lectionn�e.
		class Classe_Disquette* classe_disquette, // classe appelante.
		Infos_Secteurs_Piste_brute_16ko* Resultat);


};
// ==============================================


#endif
