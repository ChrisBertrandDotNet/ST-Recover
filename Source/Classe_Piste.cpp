//---------------------------------------------------------------------------
// Copyright Christophe Bertrand.
// This file is licensed under the Microsoft Reciprocal License (Ms-RL). You can find a copy of this license in the file "Ms-RL License.htm", in the root directory of the distribution archive.
//---------------------------------------------------------------------------


#pragma hdrstop


#include "Classe_Piste.h"


//---------------------------------------------------------------------------

#pragma package(smart_init)

//---------------------------------------------------------------------------

#include <time.h>


	#define CP_NOMBRE_ESSAIS_DE_LECTURES_DE_SECTEURS (60)

	#define CLDIS_Duree_octet_brut_en_microsecondes (32) // 200000 micros./6250 octets par piste.
	#define CLDIS_Nb_minimal_octets_entre_secteurs (54) // normal: 102. 54, c'est pour les formatages en 11 secteurs/piste, � lire en 2 tours !






	#pragma pack(push) // All msdos data structures must be packed on a 1 byte boundary
	#pragma pack (1)
	struct
	{
		DWORD StartingSector ;
		WORD NumberOfSectors ;
		DWORD pBuffer;
	}ControlBlock;
	typedef struct _DIOC_REGISTERS
	{
			DWORD reg_EBX;
			DWORD reg_EDX;
			DWORD reg_ECX;
			DWORD reg_EAX;
			DWORD reg_EDI;
			DWORD reg_ESI;
			DWORD reg_Flags;
	} DIOC_REGISTERS ;
	#pragma pack(pop)

//---------------------------------------------------------------------------

Classe_Piste::Classe_Piste(int Numero_de_piste_base_0, int Numero_de_face_base_0)
{
	Piste_base0=Numero_de_piste_base_0;
	Face_base0=Numero_de_face_base_0;

	memset(&contenu_secteurs,0,sizeof(contenu_secteurs));

	memset(&Tableau_des_Secteurs_en_base_0,0,sizeof(Tableau_des_Secteurs_en_base_0));


	sauvegarde_infos_pistes_brutes_deja_effectuee=false;
}

//---------------------------------------------------------------------------
Classe_Piste::~Classe_Piste()
{
}
//---------------------------------------------------------------------------
int	Classe_Piste::get_Numero_de_piste_base_0(void)
{
	return Piste_base0;
}
//---------------------------------------------------------------------------
int	Classe_Piste::get_Numero_de_face_base_0(void)
{
	return Face_base0;
}
//---------------------------------------------------------------------------
bool	Classe_Piste::CP_identifie_secteurs_bruts( // Renvoie si "secteur_base0" a �t� trouv�.
	class Classe_Disquette* classe_disquette, // classe appelante.
	unsigned piste, // tous les arguments sont en base 0.
	unsigned face,
	unsigned secteur_base0,
	TStrings* LOG_strings,	
	Infos_Secteurs_Piste_brute_16ko &Resultat_Piste_Brute,
	BYTE* p_memoire_secteur) // L� o� on copiera AUSSI les donn�es du secteur recherch�.

{
	bool valeur_renvoi=false;

	// Ici, j'appelle "piste brute" les octets lus en RAW,
	// et "Piste Timer" les donn�es temporelles sur la piste.

	// 3) Cherche o� se trouvent les secteurs sur la piste Timer.
	infopiste* piste_timer=CP_Analyse_Temps_Secteurs(classe_disquette,piste,face);
	if (piste_timer->OperationReussie)
	{

		// a) On d�termine le 1er secteur commun aux 2 pistes (brute et timer).
		bool sec_commun_trouve=false;
		unsigned isect_brute,isect_timer;
		{
			for (isect_brute=0;isect_brute<Resultat_Piste_Brute.Nb_Secteurs_trouves ;isect_brute++ )
			{
				if (Resultat_Piste_Brute.Infos_Secteur[isect_brute].Secteur_identifie)
				{
					for (isect_timer=0;isect_timer<(unsigned)piste_timer->fdrawcmd_Timed_Scan_Result->count ;isect_timer++ )
					{
						if (Resultat_Piste_Brute.Infos_Secteur[isect_brute].ID_Secteur_base1
						== (unsigned)piste_timer->fdrawcmd_Timed_Scan_Result->Headers[isect_timer].sector)
						{
							sec_commun_trouve=true;
							break;
						}
					}
					if (sec_commun_trouve)
						break;
				}
			}
		}
		if (sec_commun_trouve)
		{

			// b)	On calcule la longueur r�elle de la piste brute, en octets.
			//    Car la longueur id�ale est de 6250, mais sur mon lecteur elle fait 6285.
			unsigned	taille_piste_brute=6250; // valeur par d�faut.
			unsigned	duree_piste_brute=6250*CLDIS_Duree_octet_brut_en_microsecondes; // valeur par d�faut.
			{
				// Pour �a, il faut trouver 2 fois le m�me secteur dans la piste brute.
				bool couple_trouve=false;
				for (unsigned i1=0;i1<Resultat_Piste_Brute.Nb_Secteurs_trouves ;i1++ )
				{
					if (Resultat_Piste_Brute.Infos_Secteur[i1].Secteur_identifie)
					{
						for (unsigned i2=i1+1;i2<Resultat_Piste_Brute.Nb_Secteurs_trouves ;i2++ )
						{
							if (Resultat_Piste_Brute.Infos_Secteur[i2].Secteur_identifie)
							{
								if (Resultat_Piste_Brute.Infos_Secteur[i1].ID_Secteur_base1
								== Resultat_Piste_Brute.Infos_Secteur[i2].ID_Secteur_base1)
								{
									couple_trouve=true;
									int diff=
										Resultat_Piste_Brute.Infos_Secteur[i2].Index_Dans_Contenu_Piste_Codee
										- Resultat_Piste_Brute.Infos_Secteur[i1].Index_Dans_Contenu_Piste_Codee;
									const int tmaximaxi=(6250*6)/5;// tol�rance: 20%.
									if (diff>tmaximaxi) // 6250+20%
										if (diff>(tmaximaxi*2)) // 6250*2+20%
											diff /= 3; // on a fait trois tours de piste.
										else
											diff /= 2; // on a fait deux tours de piste.*/
									if ((diff<6125) || (diff > 6375)) // garde-fou (le moteur ne tourne peut-�tre pas � la bonne vitesse).
									{
										#ifdef _DEBUG
											DebugBreak();
										#endif
										diff=6250;
									}
									taille_piste_brute=diff;
									duree_piste_brute=diff*CLDIS_Duree_octet_brut_en_microsecondes;
									break;
								}
							}
						} // fin for i2
						if (couple_trouve)
							break;
					}
				} // fin for i1
			}  // fin bloc

			// - - - - - -

			// c) On calcule � quel instant a commenc� la lecture dans la piste brute.
			//		Car elle est lue � partir des 1eres donn�es de syncro trouv�es,
			//		et non au d�but physique de la piste (contrairement au timer).				<<==  TR�S IMPORTANT � NOTER !!!
			int temps_depart_piste_brute;
			{
				temps_depart_piste_brute=
					piste_timer->fdrawcmd_Timed_Scan_Result->Headers[isect_timer].reltime
					- Resultat_Piste_Brute.Infos_Secteur[isect_brute].Index_Dans_Contenu_Piste_Codee
						* CLDIS_Duree_octet_brut_en_microsecondes;
			}
			if (temps_depart_piste_brute < 0)
				temps_depart_piste_brute =
					(temps_depart_piste_brute + 3*duree_piste_brute) % duree_piste_brute;



			// -----------------------------------

			std::vector<struct bloc>	tab_blocs;
			tab_blocs.reserve(CP_NB_MAXI_SECTEURS_PAR_PISTE_BRUTE);

			Init_et_complete_tableau_blocs_zones(classe_disquette,&tab_blocs,piste_timer,LOG_strings,duree_piste_brute);

			// -----------------------------------



			// � partir d'ici, on doit utiliser "tab_blocs", au lieu
			// de "piste_timer" (en gros, contient les m�mes informations, mais avec les secteurs d�couverts en plus).


			// d) On construit un tableau des secteurs trouv�s, en rassemblant
			//		toutes les informations disponibles dans les 2 pistes.
			// Pour �a, on d�coupe le temps en zones de 4096 micro-secondes (soit 128 octets).
			unsigned zones_secteur_base1[ 8192/128 ];
			memset(zones_secteur_base1,0,sizeof(zones_secteur_base1));

			if (tab_blocs.size() > 0)
			{
				const unsigned duree_une_zone_en_microsecondes=
					128*CLDIS_Duree_octet_brut_en_microsecondes;
				const int duree_secteur=
					(128<<tab_blocs[0].taille_secteur_en_octets_code_bits)
					* CLDIS_Duree_octet_brut_en_microsecondes;

				// d1) On utilise la piste temporelle (prioritaire), avec les secteurs compl�t�s par calcul.
				for (isect_timer=0;isect_timer<
						(unsigned)tab_blocs.size(); //piste_timer_A_REMPLACER->fdrawcmd_Timed_Scan_Result->count ;
						isect_timer++ )
				{
					const unsigned nsect_base1=
						tab_blocs[isect_timer].numero_secteur_base1_en_2eme ;
					if (nsect_base1 != 0)
					{
						const unsigned instant_depart=
							tab_blocs[isect_timer].instant_depart + tab_blocs[isect_timer].duree_espace_libre_en_1er;
						const unsigned instant_fin= instant_depart
							+ duree_secteur;

						for (
							unsigned izone=instant_depart/duree_une_zone_en_microsecondes;
							izone<instant_fin/duree_une_zone_en_microsecondes ;
							izone++ )
						{
							zones_secteur_base1[izone]=nsect_base1;
						}
					}
				} // next isect_timer

				// d2) On donne le num�ro de secteur a ceux qui manquent d'ID dans la piste brute.
				{
					for (unsigned i3=0;i3<Resultat_Piste_Brute.Nb_Secteurs_trouves ;i3++ )
					{
						if ( ! Resultat_Piste_Brute.Infos_Secteur[i3].Secteur_identifie)
						{
							const unsigned octet_debut=
								Resultat_Piste_Brute.Infos_Secteur[i3].Index_Dans_Contenu_Piste_Codee
								% taille_piste_brute;
							// je compte pour un secteur de 128 octets car on ne sait pas sa taille.
							const unsigned temps_milieu_secteur=
								((octet_debut+/*128*/classe_disquette->NbOctetsParSecteur/2) * CLDIS_Duree_octet_brut_en_microsecondes
								+ temps_depart_piste_brute)
								% duree_piste_brute;
							unsigned numbase1=
								zones_secteur_base1[temps_milieu_secteur/duree_une_zone_en_microsecondes];
							if (numbase1 != 0) // si on a trouv� le num�ro du secteur (en base 1).
							{
								Resultat_Piste_Brute.Infos_Secteur[i3].ID_Secteur_base1=numbase1;
								Resultat_Piste_Brute.Infos_Secteur[i3].Secteur_identifie=true;
								Resultat_Piste_Brute.Infos_Secteur[i3].ID_Piste=this->Piste_base0;
								Resultat_Piste_Brute.Infos_Secteur[i3].ID_Face=this->Face_base0;
								Resultat_Piste_Brute.Infos_Secteur[i3].ID_Taille=
									Ln((double)classe_disquette->NbOctetsParSecteur/128.0)/Ln(2.0);
							}
							else
							{
								#ifdef _DEBUG
									DebugBreak();
								#endif
								LOG_strings->Add("Warning: sector not identified. There is a default in the sector scheme analysis. Please contact the author.");
							}

							// ENFIN on est au point o� on peut utiliser des secteurs anonymes.
							// ----------------------------------------------------------------

							if (Resultat_Piste_Brute.Infos_Secteur[i3].Secteur_identifie)
							{
								bool cestlesecteurrecherche=
									Resultat_Piste_Brute.Infos_Secteur[i3].ID_Secteur_base1 == (secteur_base0+1);
								// Ici, je devrais aussi tester les ID de piste,face et taille.
								// Mais �a risque de g�ner la lecture des disquettes sp�ciales.

								s_Secteur* sectparbrut=
									&Tableau_des_Secteurs_en_base_0[
										Resultat_Piste_Brute.Infos_Secteur[i3].ID_Secteur_base1 -1 ];

								if ( ! sectparbrut->Lu_correctement)
								{
									BYTE* mem=
										cestlesecteurrecherche ?
											p_memoire_secteur
											: reserve_memoire_secteur(
													Resultat_Piste_Brute.Infos_Secteur[i3].ID_Secteur_base1 -1,
													classe_disquette->NbOctetsParSecteur);

									if (mem != NULL)
									{
										memcpy(mem,&Resultat_Piste_Brute.Contenu_Piste_decodee[
											Resultat_Piste_Brute.Infos_Secteur[i3].Index_Dans_Contenu_Piste_Decodee],
											classe_disquette->NbOctetsParSecteur);
										sectparbrut->Lu_correctement=true;
										sectparbrut->Taille_en_octets=classe_disquette->NbOctetsParSecteur;
										sectparbrut->Lecture_par_Piste_Brute_reussie=true;
										sectparbrut->Nombre_essais_lecture ++;
										if (cestlesecteurrecherche)
										{
											valeur_renvoi=true;
											//break; // "break" si on se limite � notre secteur actuellement recherch� (debug).
										}
									}
								}
							}

						}
					}
				}
			}
			tab_blocs.clear();
		}
	}  // endif piste_timer->OperationReussie
	return valeur_renvoi;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool	Classe_Piste::CP_LitSecteur(
	Classe_Disquette* classe_disquette, // classe appelante.
	unsigned piste, // tous les arguments sont en base 0.
	unsigned face,
	unsigned secteur_base0,
	DWORD temps_alloue_ms,
	s_Secteur** p_p_s_secteur, // Ecrit un pointeur sur une classe dans la m�moire fournie.
	TStrings* LOG_strings,
	volatile bool*	p_annulateur, // si cette variable devient vraie, on annule l'op�ration en cours.
	bool sauve_infos_pistes_brutes)
{
	const DWORD limite_temps=GetTickCount()+temps_alloue_ms;

	s_Secteur* secteur=&Tableau_des_Secteurs_en_base_0[secteur_base0];

	*p_p_s_secteur=secteur;

	if (secteur->Lu_correctement) // Cas o� ce secteur a d�j� �t� lu.
		return true;

	if (*p_annulateur)
		return false;



	const bool determine_architecture= classe_disquette->ArchitectureDisqueConnue;
	BYTE*	p_memoire_secteur=
		determine_architecture ?
		reserve_memoire_secteur(secteur_base0,classe_disquette->NbOctetsParSecteur)
		: (BYTE*) &classe_disquette->Secteur_Boot_Atari_ST;//_memoire_secteur : // si on ne connait pas l'archi, on �crit dans cette m�moire.

	secteur->pContenu=p_memoire_secteur; // M�me pour le boot-secteur, qui est assez sp�cial et limit� � 512 octets.

	bool OK=false;

	const unsigned startinglogicalsector =
		( (!classe_disquette->ArchitectureDisqueConnue) || ((piste==0) && (face==0) && (secteur_base0==0))) ?
			0 :
			(piste*classe_disquette->NbSecteursParPiste*classe_disquette->NbFaces + face*classe_disquette->NbSecteursParPiste)
			+ secteur_base0;

	const numberofsectors = 1;

	if (classe_disquette->Win9X)
	{
		// code for win 95/98
		ControlBlock.StartingSector = (DWORD)startinglogicalsector;
		ControlBlock.NumberOfSectors = (DWORD)numberofsectors ;
		ControlBlock.pBuffer = (DWORD)p_memoire_secteur;

		//-----------------------------------------------------------
		// SI contains read/write mode flags
		// SI=0h for read and SI=1h for write
		// CX must be equal to ffffh for
		// int 21h's 7305h extention
		// DS:BX -> base addr of the
		// control block structure
		// DL must contain the drive number
		// (01h=A:, 02h=B: etc)
		//-----------------------------------------------------------

    DIOC_REGISTERS reg ;

		reg.reg_ESI = 0x00 ;
		reg.reg_ECX = -1 ;
		reg.reg_EBX = (DWORD)(&ControlBlock);
		reg.reg_EDX = classe_disquette->LecteurSelectionne+1;
		reg.reg_EAX = 0x7305 ;

		//  6 == VWIN32_DIOC_DOS_DRIVEINFO
		BOOL  fResult ;
		DWORD cb ;

		fResult = DeviceIoControl ( classe_disquette->hDevice,
			6,
			&(reg),
			sizeof (reg),
			&(reg),
			sizeof (reg),
			&cb,
			0);

		 OK = ! (!fResult || (reg.reg_Flags & 0x0001));
	}
	else
	{       // Code Windows NT

		if ( ! classe_disquette->fdrawcmd_sys_installe)
		{
			// Setting the pointer to point to the start of the sector we want to read ..
			SetFilePointer (classe_disquette->hDevice, (startinglogicalsector*classe_disquette->NbOctetsParSecteur), NULL, FILE_BEGIN);

			DWORD bytesread=0;
			OK=ReadFile (classe_disquette->hDevice, p_memoire_secteur, classe_disquette->NbOctetsParSecteur*numberofsectors, &bytesread, NULL);
			secteur->Lecture_normale_par_controleur_essayee=true;
			if (OK)
			{
				secteur->Lu_correctement=true;
				secteur->Nombre_essais_lecture++;
				secteur->Taille_en_octets = bytesread;
				secteur->Lecture_normale_par_controleur_essayee=true;
				secteur->Lecture_normale_par_controleur_reussie=true;
			}
		}
		else
		{
			// Avec ce pilote (fdrawcmd.sys), on est forc� d'utiliser une autre m�thode.
			DWORD dwRet;
			FD_READ_WRITE_PARAMS rwp;

			// details of sector to read
			rwp.flags = FD_OPTION_MFM;
			rwp.phead = face;
			rwp.cyl = piste;
			rwp.head = face;
			rwp.sector = secteur_base0+1; // en base 1.
			rwp.size = 2;
			rwp.eot = secteur_base0+1+1; // secteur 'suivant' le dernier � lire, en base 1.
			rwp.gap = 0x0a;
			rwp.datalen = 0xff;

			OK = CP_selectionne_piste_et_face(classe_disquette,piste,face);
			if (OK)
			{
				classe_disquette->infos_en_direct.Piste_Selectionnee=piste; // La t�te est actuellement plac�e ici.
				classe_disquette->infos_en_direct.Face_Selectionnee=face; // Cette face est actuellement selectionn�e.

				bool secteurlu=false;
				int nb_essais_ici=0;


				// -------------------------------------------------------------
				while((!secteurlu)
					&& (((int)limite_temps-GetTickCount())>0)
					&& ( ! *p_annulateur)
					&& ( nb_essais_ici < CP_NOMBRE_ESSAIS_DE_LECTURES_DE_SECTEURS) )
				{
					if ((nb_essais_ici % 16)==15) // Toutes les 16 boucles, on recalibre la t�te.. �  tout hasard.
					{
						DeviceIoControl(classe_disquette->hDevice, IOCTL_FDCMD_RECALIBRATE, NULL, 0, NULL, 0, &dwRet, NULL);
						// seek to cyl "piste"
						CP_selectionne_piste_et_face(classe_disquette,piste,face);
					}


					// read sector
					secteurlu=DeviceIoControl(classe_disquette->hDevice, IOCTL_FDCMD_READ_DATA, &rwp,
						sizeof(rwp),p_memoire_secteur, classe_disquette->NbOctetsParSecteur, &dwRet, NULL);

					secteur->Nombre_essais_lecture ++;
					secteur->Lecture_normale_par_controleur_essayee=true; // TRES IMPORTANT: on met 'vrai', que la lecture ait r�ussi ou pas.
					secteur->Lecture_normale_par_controleur_reussie=secteurlu;
					secteur->Lu_correctement |= secteurlu;

					if ( ! secteurlu )
					{
						// Ici on demande une technique plus approfondie pour lire ce secteur.

						struct Infos_Secteurs_Piste_brute_16ko Resultat_Piste_Brute;

						bool recup_ok=CP_analyse_piste_brute( // seulement avec "fdrawcmd.sys".
							false,//bool Purement_informatif,// Ne modifie pas les donn�es de secteur.
							//out_Infos_detaillees,//TStringList* out_Infos_detaillees,
							classe_disquette,//Classe_Disquette* classe_disquette, // classe appelante.
							limite_temps-GetTickCount(),//DWORD temps_alloue_ms,
							p_p_s_secteur,//s_Secteur** p_p_s_secteur, // Ecrit un pointeur sur une classe dans la m�moire fournie.
							LOG_strings,//TStrings* LOG_strings,
							p_annulateur,//volatile bool*	p_annulateur, // si cette variable devient vraie, on annule l'op�ration en cours.
							sauve_infos_pistes_brutes,//bool sauve_infos_pistes_brutes);
							&Resultat_Piste_Brute);



						// Cherche si on trouve notre secteur dans la piste brute.
						if (recup_ok)
						{
							// Cherche si notre secteur a �t� bien lu.
							for (unsigned i=0; i<Resultat_Piste_Brute.Nb_Secteurs_trouves;i++ )
							{
								if (Resultat_Piste_Brute.Infos_Secteur[i].Secteur_identifie)
								{
									const bool cestlesecteurrecherche=
										Resultat_Piste_Brute.Infos_Secteur[i].ID_Secteur_base1 == (secteur_base0+1);
									// Ici, je devrais aussi tester les ID de piste,face et taille.
									// Mais �a risque de g�ner la lecture des disquettes sp�ciales.

									s_Secteur* sectparbrut=
										&Tableau_des_Secteurs_en_base_0[
											Resultat_Piste_Brute.Infos_Secteur[i].ID_Secteur_base1 -1 ];


									if (sectparbrut->Lu_correctement)
									{
										if (cestlesecteurrecherche)
											secteurlu=true;
									}
									else
									{
										BYTE* mem=
											cestlesecteurrecherche ?
												p_memoire_secteur
												: reserve_memoire_secteur(
														Resultat_Piste_Brute.Infos_Secteur[i].ID_Secteur_base1 -1,//unsigned secteur_base0,
														classe_disquette->NbOctetsParSecteur);//unsigned nombre_octets);

										if (mem != NULL)
										{
											memcpy(mem,&Resultat_Piste_Brute.Contenu_Piste_decodee[
												Resultat_Piste_Brute.Infos_Secteur[i].Index_Dans_Contenu_Piste_Decodee],
												classe_disquette->NbOctetsParSecteur);
											sectparbrut->Lu_correctement=true;
											sectparbrut->Taille_en_octets=classe_disquette->NbOctetsParSecteur;
											sectparbrut->Lecture_par_Piste_Brute_reussie=true;
											sectparbrut->Nombre_essais_lecture ++;
											if (cestlesecteurrecherche)
											{
												secteurlu=true;
												//break; // "break" si on se limite � notre secteur actuellement recherch� (debug).
											}
										}
									}
								}
							} // next i
						} // endif (recup_ok)

					} // endif ( ! secteurlu || (sauvetoujourspistebrut && (nb_essais_ici==0)))
					nb_essais_ici++;
				} // end while // --------------------------------------------------

				OK &= secteurlu;

				if (secteurlu && !secteur->Lu_correctement)
					asm nop // ERREUR

				if (secteurlu && (!determine_architecture))
				{		// On avait stock� le contenu dans une m�moire tempo, il faut la copier.
					BYTE* p=reserve_memoire_secteur(secteur_base0,classe_disquette->NbOctetsParSecteur);
					OK &= (p != NULL);
					if (p != NULL)
					{
						memcpy(p,p_memoire_secteur,classe_disquette->NbOctetsParSecteur);
						secteur->pContenu=p;
					}
				} // endif (secteurlu && (!determine_architecture))
			} // endif DeviceIoControl(classe_disquette->hDevice, IOCTL_FDCMD_SEEK, &sp, sizeof(sp), NULL, 0, &dwRet, NULL);
		} // endif ( ! classe_disquette->fdrawcmd_sys_installe)
	} // endif (classe_disquette->Win9X)

	return OK;
}
//---------------------------------------------------------------------------
bool	Classe_Piste::CP_selectionne_piste_et_face(
	Classe_Disquette* classe_disquette, // classe appelante.
	unsigned piste, // en base 0.
	unsigned face) // en base 0.
{
	if (classe_disquette==NULL)
		return false;
	if ( ! classe_disquette->fdrawcmd_sys_installe)
		return false;

	// Avec ce pilote (fdrawcmd.sys), on est forc� d'utiliser une autre m�thode.
	DWORD dwRet;
	FD_READ_WRITE_PARAMS rwp;
	FD_SEEK_PARAMS sp;

	// details of sector to read
	rwp.flags = FD_OPTION_MFM;
	rwp.phead = face;
	rwp.cyl = piste;
	rwp.head = face;
	rwp.sector = 1; // en base 1.
	rwp.size = 2;
	rwp.eot = 1+1; // secteur 'suivant' le dernier � lire, en base 1.
	rwp.gap = 0x0a;
	rwp.datalen = 0xff;

	// details of seek location
	sp.cyl = piste;
	sp.head = face;

	// seek to cyl "piste"
	return DeviceIoControl(classe_disquette->hDevice, IOCTL_FDCMD_SEEK, &sp, sizeof(sp), NULL, 0, &dwRet, NULL);
}
//---------------------------------------------------------------------------
bool	Classe_Piste::CP_analyse_piste_brute(// on lit la piste brute.
	bool Purement_informatif,// Ne modifie pas les donn�es de secteur. Sinon, on en extrait les informations vers les secteurs.
	//TStringList* out_Infos_detaillees,
	Classe_Disquette* classe_disquette, // classe appelante.
	DWORD temps_alloue_ms,
	s_Secteur** p_p_s_secteur, // Ecrit un pointeur sur une classe dans la m�moire fournie.
	TStrings* LOG_strings,
	volatile bool*	p_annulateur, // si cette variable devient vraie, on annule l'op�ration en cours.
	bool sauve_infos_pistes_brutes, // Dans un fichier texte et un fichier binaire.
	struct Infos_Secteurs_Piste_brute_16ko* Resultat_Piste_Brute)
{
	/*	Petite explication:

	Il y deux raisons d'appeler cette fonction.

	- Soit on ne parvient pas � lire un des secteurs, et on a besoin d'une meilleure
	analyse. Dans ce cas, "Purement_informatif"="false", et on va renseigner
	les informations des secteurs manquants.

	- Soit on veut lancer une analyse approfondie de la piste APRES la lecture normale
	des secteurs, pour fournir des informations de d�bogage.
	Dans ce cas, "Purement_informatif"="true", et on ne va garder les informations
	sur les secteurs. Par contre, on va �crire un fichier d'information de la piste.
	A noter, TRES IMPORTANT, que m�me dans ce dernier cas, il est possible que l'on
	ait d�j� appel� cette fonction avant (pour la m�me piste), � cause d'une difficult�
	� lire un secteur.
	*/


	if (*p_annulateur)
		return false;
	if ( ! classe_disquette->fdrawcmd_sys_installe)
		return false;

	if (Resultat_Piste_Brute==NULL)
		return false;
	memset(Resultat_Piste_Brute,0,sizeof(Infos_Secteurs_Piste_brute_16ko));

	bool OK=false;

	OK= CP_selectionne_piste_et_face(classe_disquette,Piste_base0,Face_base0);

	if (!OK)
		return false;

	classe_disquette->infos_en_direct.Piste_Selectionnee=Piste_base0; // La t�te est actuellement plac�e ici.
	classe_disquette->infos_en_direct.Face_Selectionnee=Face_base0; // Cette face est actuellement selectionn�e.

	int nb_essais_ici=0;


	// -------------------------------------------------------------

			bool recup_ok;

			// 1) lit la piste brute, pour y d�nicher des secteurs.
			recup_ok=LitSecteursPisteBrute(classe_disquette,Resultat_Piste_Brute);
			if ( ! recup_ok)
				return false;

			{
				// On va t�cher de situer aussi les secteurs pas encore trouv�s.

				// On regarde si on a besoin d'aller plus loin.
				int nb_secteurs_bruts_sans_ID=0;
				for (unsigned i=0; i<Resultat_Piste_Brute->Nb_Secteurs_trouves;i++ )
					if ( ! Resultat_Piste_Brute->Infos_Secteur[i].Secteur_identifie)
						nb_secteurs_bruts_sans_ID++;

				if (nb_secteurs_bruts_sans_ID > 0)
					CP_identifie_secteurs_bruts( // **************************
						classe_disquette,//class Classe_Disquette* classe_disquette, // classe appelante.
						Piste_base0,//unsigned piste, // tous les arguments sont en base 0.
						Face_base0,//unsigned face,
						1000,// secteur_base0 AUCUNE IMPORTANCE ICI.
						LOG_strings,//TStrings* LOG_strings,
						*Resultat_Piste_Brute,//Infos_Secteurs_Piste_brute_16ko &Resultat_Piste_Brute,
						NULL);//p_memoire_secteur);//BYTE* p_memoire_secteur); // L� o� on copiera AUSSI les donn�es du secteur recherch�.


				// pour les essais plus approfondis: la sauvegarde de la piste brute:
				// (Peut aussi servir aux utilisateurs qui rencontrent un probl�me).

				if (sauve_infos_pistes_brutes)
				{
					//if ( ! sauvegarde_infos_pistes_brutes_deja_effectuee) // Avec cette v�rif, on ne garde qu'une trace d'analyse, ce qui me semble insuffisant.
					{
						sauvegarde_infos_pistes_brutes_deja_effectuee=true;

						AnsiString nom,s;
						{
							time_t tt;
							time(&tt);
							struct tm* t=localtime(&tt);

							if (t != NULL)
							{
								nom.printf("%04d-%02d-%02d %02d.%02d.%02d ",
									t->tm_year+1900,t->tm_mon+1,t->tm_mday,
									t->tm_hour,t->tm_min,t->tm_sec);
							}
						}
						s.printf("Raw track (track %02d, head %d)",Piste_base0,Face_base0);
						nom = nom+s;

						s=nom+".floppy_raw_track";
						HANDLE hf=CreateFile(
							s.c_str(),//LPCTSTR lpFileName,
							GENERIC_WRITE,//DWORD dwDesiredAccess,
							FILE_SHARE_READ,//DWORD dwShareMode,
							NULL,//LPSECURITY_ATTRIBUTES lpSecurityAttributes,
							CREATE_ALWAYS,//DWORD dwCreationDisposition,
							FILE_ATTRIBUTE_NORMAL,//DWORD dwFlagsAndAttributes,
							NULL);//HANDLE hTemplateFile
						if (hf == INVALID_HANDLE_VALUE)
						{
							//DWORD dwerr=GetLastError();
							//asm nop
						}
						else
						{
							DWORD e;
							const BOOL okf=WriteFile(
								hf,//HANDLE hFile,
								Resultat_Piste_Brute->Contenu_Piste_codee ,//LPCVOID lpBuffer,
								sizeof(Resultat_Piste_Brute->Contenu_Piste_codee) ,//DWORD nNumberOfBytesToWrite,
								&e,//LPDWORD lpNumberOfBytesWritten,
								NULL);//LPOVERLAPPED lpOverlapped
							if (! okf)
							{
								//DWORD dwerr=GetLastError();
								asm nop
							}
							else
							{ // on sauve aussi les informations utiles:
								TStringList* l=new TStringList();
								l->Add("Raw track information, created by ST Recover.");
								s.printf("Track %d - Head %d",Piste_base0,Face_base0);
								l->Add(s);
								//s.printf("Looking for sector (base 1): %d .. and cannot find it.",secteur_base0+1);
								//l->Add(s);
								//s.printf("Relative beginning of track in octets: %d",-);
								l->Add("List of found sectors:");
								const char tt[2][4]={"No","Yes"};
								for (unsigned is=0;is<Resultat_Piste_Brute->Nb_Secteurs_trouves;is++)
								{
									s.printf("Index in raw octets:%d (0x%X) - Identified sector ? %s - ID found ? %s - ID track:%d ID head:%d ID sector_base1:%d ID size bits:%d",
									Resultat_Piste_Brute->Infos_Secteur[is].Index_Dans_Contenu_Piste_Codee,
									Resultat_Piste_Brute->Infos_Secteur[is].Index_Dans_Contenu_Piste_Codee,
									tt[Resultat_Piste_Brute->Infos_Secteur[is].Secteur_identifie & 1],
									tt[Resultat_Piste_Brute->Infos_Secteur[is].ID_trouve_directement & 1],
									Resultat_Piste_Brute->Infos_Secteur[is].ID_Piste,
									Resultat_Piste_Brute->Infos_Secteur[is].ID_Face,
									Resultat_Piste_Brute->Infos_Secteur[is].ID_Secteur_base1,
									Resultat_Piste_Brute->Infos_Secteur[is].ID_Taille);
									l->Add(s);
								}

								s=nom+" - Informations.txt";
								l->SaveToFile(s);
								delete l;
							}

							CloseHandle(hf);
						} // endif (hf == INVALID_HANDLE_VALUE)
					} // endif ( ! dejasauve)
				}  // endif sauve_infos_pistes_brutes
			} // endif if ((! secteurlu) || (sauvetoujourspistebrut && (nb_essais_ici==0)))

		nb_essais_ici++;

	return OK;
}
//---------------------------------------------------------------------------

BYTE* Classe_Piste::reserve_memoire_secteur(unsigned secteur_base0, unsigned nombre_octets)
{
	// on v�rifie si ce secteur a d�j� une m�moire attribu�e.
	if (contenu_secteurs.index_memoire_secteurs_base1[secteur_base0+1] != 0)
		return & contenu_secteurs.memoire[contenu_secteurs.index_memoire_secteurs_base1[secteur_base0+1]];

	// Maintenant on r�serve une m�moire.

	if ((contenu_secteurs.index_memoire_libre+nombre_octets)
			> sizeof(contenu_secteurs.memoire))
		return NULL; // plus assez de m�moire dispo. Ce qui serait tr�s �trange !

	// On �vite de commencer au d�but, sinon le 1er index sera z�ro, ce qui poserait un probl�me plus haut.
	if (contenu_secteurs.index_memoire_libre < 16)
		contenu_secteurs.index_memoire_libre=16;

	if ( (secteur_base0+1+1)
	 > (sizeof(contenu_secteurs.index_memoire_secteurs_base1)
			/ sizeof(contenu_secteurs.index_memoire_secteurs_base1[0])) )
		return NULL; // plus assez d'indexs dispo.

	contenu_secteurs.index_memoire_secteurs_base1[secteur_base0+1]=contenu_secteurs.index_memoire_libre;
	contenu_secteurs.index_memoire_libre += nombre_octets;

	// Met aussi � jour le Tableau_des_Secteurs.
	s_Secteur* s=
		&Tableau_des_Secteurs_en_base_0[secteur_base0];
	s->pContenu = & contenu_secteurs.memoire[contenu_secteurs.index_memoire_secteurs_base1[secteur_base0+1]];
	s->Taille_en_octets=nombre_octets;

	return s->pContenu;
}
// ====================================
// ---------------------------------------------------------------------------------

bool	Classe_Piste::Init_et_complete_tableau_blocs_zones(
	Classe_Disquette* classe_disquette, // classe appelante.
	std::vector<struct bloc>*	tableau_blocs,
	infopiste* piste_timer,
	TStrings* LOG_strings,
	int duree_piste_1_tour) // en microsecondes.
{
	// on recherche les espaces vides pouvant abriter un secteur.

	if (tableau_blocs==NULL)
		return false;

	bool valrenvoi=false;

	{
		// d'abord on d�termine la taille des secteurs, et leur s�paration.
		int	taille_secteurs=512;
		if (piste_timer->fdrawcmd_Timed_Scan_Result->count != 0)
			taille_secteurs=128 << piste_timer->fdrawcmd_Timed_Scan_Result->Headers[0].size;//par d�faut
		int duree_entre_secteurs=
			(CLDIS_Nb_minimal_octets_entre_secteurs+taille_secteurs)
			* CLDIS_Duree_octet_brut_en_microsecondes;//minimum par d�faut.
		int nb_intersecteurs=0;
		int total_duree_entre_secteurs=0;
		bool	secteurs_de_taille_uniforme;
		{
			int tmini=1000000;
			int tmaxi=-1;
			int instant_secteur_precedent=0;
			for (int isect_timer=0;isect_timer<piste_timer->fdrawcmd_Timed_Scan_Result->count ;isect_timer++ )
			{
				const int t=
					128 << piste_timer->fdrawcmd_Timed_Scan_Result->Headers[isect_timer].size;
				if (t>tmaxi)
					tmaxi=t;
				if (t<tmini)
					tmini=t;
				int i=piste_timer->fdrawcmd_Timed_Scan_Result->Headers[isect_timer].reltime;
				if ( ((i-instant_secteur_precedent) > (CLDIS_Nb_minimal_octets_entre_secteurs*CLDIS_Duree_octet_brut_en_microsecondes))
					&& ((i-instant_secteur_precedent) <= (((t*3)/2+CLDIS_Nb_minimal_octets_entre_secteurs)*CLDIS_Duree_octet_brut_en_microsecondes))
					&& (isect_timer >= 2) )
				{
					total_duree_entre_secteurs +=
						i-instant_secteur_precedent - (t*CLDIS_Duree_octet_brut_en_microsecondes);
					nb_intersecteurs++;
				}

				instant_secteur_precedent=i;
			} // next isect_timer
			secteurs_de_taille_uniforme=(tmini==tmaxi);
			taille_secteurs=tmini;
			if (nb_intersecteurs != 0)
				duree_entre_secteurs = total_duree_entre_secteurs / nb_intersecteurs;
		}
		//if (secteurs_de_taille_uniforme)
		{			// On ne peut appliquer cet algo que si les secteurs sont tous de m�me taille.
					// Et aussi qu'on a trouv� au moins 1 secteur.
			int instant_fin_secteur_precedent=0;

			for (int isect_timer=0;isect_timer<piste_timer->fdrawcmd_Timed_Scan_Result->count ;isect_timer++ )
			{
				bloc bl;
				bl.instant_depart=instant_fin_secteur_precedent;
				bl.duree_espace_libre_en_1er =
					piste_timer->fdrawcmd_Timed_Scan_Result->Headers[isect_timer].reltime
					- bl.instant_depart;
				bl.numero_secteur_base1_en_2eme=
					piste_timer->fdrawcmd_Timed_Scan_Result->Headers[isect_timer].sector;
				bl.taille_secteur_en_octets_code_bits=
					piste_timer->fdrawcmd_Timed_Scan_Result->Headers[isect_timer].size;
				bl.trouve_par_le_controleur=true;

				tableau_blocs->push_back(bl);
				instant_fin_secteur_precedent =
					piste_timer->fdrawcmd_Timed_Scan_Result->Headers[isect_timer].reltime
					+ (128<<piste_timer->fdrawcmd_Timed_Scan_Result->Headers[isect_timer].size)
						* CLDIS_Duree_octet_brut_en_microsecondes; //duree_secteur;
				// On en profite pour calculer la moyenne des espaces normaux entre des secteurs.
				// On ne compte �videmment pas les vides laiss�s par les secteurs non d�tect�s.
				/*int intersecteur=bl.duree_espace_libre_en_1er;
				if ((intersecteur>=duree_minimale_entre_secteurs)
					&& (intersecteur<=duree_maximale_entre_secteurs))
				{
					//duree_entre_secteurs += intersecteur;
					nb_intersecteurs_normaux++;
				} */
			}

			const int duree_secteur=taille_secteurs * CLDIS_Duree_octet_brut_en_microsecondes;
			const int duree_bloc=duree_secteur+duree_entre_secteurs;
			const int duree_bloc_mini=duree_bloc-12/*NB_MAX_SECTEURS_PAR_PISTE*/*CLDIS_Duree_octet_brut_en_microsecondes; // normalement, l'espacement est tr�s r�gulier.

			{    // L'espace final peut tr�s bien contenir des secteurs pas encore identifi�s.
				const int temps_restant_apres_dernier_secteur_identifie=
					duree_piste_1_tour - instant_fin_secteur_precedent;
				if (temps_restant_apres_dernier_secteur_identifie >= duree_bloc_mini)
				{
					// On cr�e un bloc sans secteur.
					bloc bl;
					bl.instant_depart=instant_fin_secteur_precedent;
					bl.duree_espace_libre_en_1er =
						duree_piste_1_tour	- bl.instant_depart;
					bl.numero_secteur_base1_en_2eme=0;
					bl.taille_secteur_en_octets_code_bits=0;
					bl.trouve_par_le_controleur=false;
          
					tableau_blocs->push_back(bl);
				}
			}

			if (secteurs_de_taille_uniforme)// && (nb_intersecteurs_normaux != 0))
			{			// On ne peut appliquer cet algo que si les secteurs sont tous de m�me taille.
						// Et aussi qu'on a trouv� au moins 1 secteur (par le controleur).
				// Etape suivante: on comble les vides avec des secteurs.
				bool secteur_trouve_par_controleur[CP_NB_MAXI_SECTEURS_PAR_PISTE_BRUTE+2];
				memset(secteur_trouve_par_controleur,false,sizeof(secteur_trouve_par_controleur));
				int nb_secteurs_nouveaux=0;
				// Ici, on ajoute des secteurs l� il y a de la place, en cr�ant de nouveaux "blocs".
				for (int izone=0;izone<(int)(tableau_blocs->size());izone++)
				{
					const int espace=(*tableau_blocs)[izone].duree_espace_libre_en_1er;
					// Le 1er secteur de la piste a un espace court avant lui, c'est normal.
					const int ajout_debut_piste= (izone==0)
						? (44-16)*CLDIS_Duree_octet_brut_en_microsecondes
						: 0;
					const int nb_sect_a_inserer=
						(espace/*-duree_entre_secteurs*/+ajout_debut_piste)
						/ duree_bloc_mini;

					// On en profite pour lister les secteurs trouv�s par le controleur.
					{
						const unsigned is=(*tableau_blocs)[izone].numero_secteur_base1_en_2eme;
						if ((is != 0) && (is <=CP_NB_MAXI_SECTEURS_PAR_PISTE_BRUTE))
							secteur_trouve_par_controleur[(*tableau_blocs)[izone].numero_secteur_base1_en_2eme]=true;
					}

					if (nb_sect_a_inserer > 0)
					{ // TODO: Ici, il faudrait tenir compte de "ajout_debut_piste".
						int instant=(*tableau_blocs)[izone].instant_depart;

						// d'abord met � jour les donn�es du secteur d�couvert.
						{
							const int diminutionespace=
								(duree_entre_secteurs/*interespaces*/+duree_secteur)*nb_sect_a_inserer;
							(*tableau_blocs)[izone].instant_depart
								+= diminutionespace;
							(*tableau_blocs)[izone].duree_espace_libre_en_1er
								-= diminutionespace;
						}
						// ensuite rempli l'espace libre.
						for (int iblocins=0;iblocins<nb_sect_a_inserer;iblocins++)
						{
							// on cr�e un secteur au d�but de l'espace libre.
							bloc bl;
							bl.instant_depart=instant;
							bl.duree_espace_libre_en_1er=duree_entre_secteurs;//interespaces;
							bl.numero_secteur_base1_en_2eme=0; // pas encore son num�ro.
							bl.taille_secteur_en_octets_code_bits=
								(*tableau_blocs)[izone].taille_secteur_en_octets_code_bits; // identiques puisque tous les secteurs ont la m�me taille.
							bl.trouve_par_le_controleur=false;
							instant += duree_entre_secteurs/*interespaces*/+duree_secteur;
							tableau_blocs->insert(tableau_blocs->begin()+izone+iblocins,bl);
							nb_secteurs_nouveaux++;
						}
						izone += nb_sect_a_inserer;// pas la peine de tester les nouveaux blocs.
					}
				}  // next izone
				/* Supprime la derni�re zone, s'il elle n'a pas la place pour contenir
						au moins un secteur. */
				if (tableau_blocs->size() != 0)
				{
					int idernier=tableau_blocs->size()-1;
					if ((*tableau_blocs)[idernier].taille_secteur_en_octets_code_bits ==0)
            tableau_blocs->pop_back(); // �limine le dernier.
				}

				// Etape finale: on num�rote les secteurs ajout�s.
				// Pour �a, on d�termine l'ordre g�n�ral.
				if (nb_secteurs_nouveaux==1)
				{
					// Ici, j'applique la m�thode la plus simple:
					//  trouver le seul secteur qui manque.
					int numero_secteur_manquant_base1=0;
					for (int is=1;is<(int)(sizeof(secteur_trouve_par_controleur)/sizeof(secteur_trouve_par_controleur[0]));is++)
					{
						if ( ! secteur_trouve_par_controleur[is])
						{
							numero_secteur_manquant_base1=is;
							for (int izone=0;(unsigned)izone<tableau_blocs->size();izone++)
							{
								if ( ! (*tableau_blocs)[izone].trouve_par_le_controleur)
								{
									(*tableau_blocs)[izone].numero_secteur_base1_en_2eme
										= numero_secteur_manquant_base1;
									break;
								}
							}
							break;
						}
					}
				}
				else
				{
					if (nb_secteurs_nouveaux != 0)  // Ici on analyse l'ordre des secteurs.
					{
						//LOG_strings->Add("Warning: more than one (1) sector discovered by raw track read.\nWe need more analysis (not developped now). Please ask to the author ;) Check 'Save track information (debug)', and send the track information files to me.");

						/* 1) d�termine l'incr�ment de num�ros de secteur
									(pas forc�ment "1", � cause des entrelacements de secteurs,
									par exemple pour les pistes � 11 secteurs, pour acc�l�rer la lecture.*/
						bool increment_trouve=false;
						int increment=1;
						const int _NbSecteursParPiste=classe_disquette->NbSecteursParPiste;
						for (int izone=1;izone<(int)tableau_blocs->size();izone++)
						{
							if ((*tableau_blocs)[izone].numero_secteur_base1_en_2eme != 0)
								if ((*tableau_blocs)[izone-1].numero_secteur_base1_en_2eme != 0)
								{
									increment =
										(((*tableau_blocs)[izone].numero_secteur_base1_en_2eme
										- (*tableau_blocs)[izone-1].numero_secteur_base1_en_2eme)
										+ _NbSecteursParPiste)
										% _NbSecteursParPiste;
									increment_trouve=true;
									break;
								}
						}

						// 2) pr�pare une liste des secteurs, en num�rotant ceux qui manquent.
						if (increment_trouve)
						{
							int numeros_secteurs_base1[CP_NB_MAXI_SECTEURS_PAR_PISTE_BRUTE+2];
							memset(numeros_secteurs_base1,0,sizeof(numeros_secteurs_base1));
							// d'abord de la fin vers le d�but, pour assurer le num�ro du 1er secteur de la piste.
							int n_precedent=0;
							for (int izone=tableau_blocs->size()-1;izone >=0;izone--)
							{
								int n=(*tableau_blocs)[izone].numero_secteur_base1_en_2eme;
								if ((n == 0) && (n_precedent !=0))
								{
									n=
										(((n_precedent-1-increment) + _NbSecteursParPiste)
										% _NbSecteursParPiste)
										+1;// "+1" car en base 1.
								}
								numeros_secteurs_base1[izone]=n;
								n_precedent=n;
							}
							// ensuite du d�but vers la fin, pour num�roter tous les secteurs jusqu'� la fin.
							n_precedent=0;
							for (int izone=0;izone<(int)tableau_blocs->size();izone++)
							{
								int n=numeros_secteurs_base1[izone];
								if ((n == 0) && (n_precedent !=0))
								{
									n=
										(((n_precedent-1+increment) + _NbSecteursParPiste)
										% _NbSecteursParPiste)
										+1;// "+1" car en base 1.
									numeros_secteurs_base1[izone]=n;
								}
								n_precedent=n;
							}

							/* 3) on construit un tableau des secteurs dans l'ordre num�rique,
										pour voir si tous ne sont pr�sents qu'une fois,
										pour voir si mon incr�ment est valable.*/
							int nombre_de_presences_secteurs_base1[CP_NB_MAXI_SECTEURS_PAR_PISTE_BRUTE+2];
							memset(nombre_de_presences_secteurs_base1,0,sizeof(nombre_de_presences_secteurs_base1));
							for (int izone=0;izone<(int)tableau_blocs->size();izone++)
							{
								int n=numeros_secteurs_base1[izone];
								if (n < CP_NB_MAXI_SECTEURS_PAR_PISTE_BRUTE)
								{
									nombre_de_presences_secteurs_base1[n]++;
								}
							}
							// chaque num�ro de secteur doit �tre pr�sent une fois et une seule.
							bool increment_valable=true;
							if (nombre_de_presences_secteurs_base1[0]!=0) // sp�cial, car normalement aucun secteur de num�ro 0 (car en base 1).
								increment_valable=false;
							else
								for (int izone=1;izone<(int)tableau_blocs->size();izone++)
								{
									if (nombre_de_presences_secteurs_base1[izone] != 1)
									{
										increment_valable=false;
										LOG_strings->Add("Warning: more than one (1) sector discovered by raw track read, AND the increment is not regular (not even regular interlaced sectors).\n We need more analysis (not developped now). Please ask to the author ;) Check 'Save track information (debug)', and send the track information files to me.");
										break;
									}
								}

							// 4) si l'incr�ment est bon, on l'applique.
							if (increment_valable)
							{
								for (int izone=0;izone<(int)tableau_blocs->size();izone++)
								{
									if ((*tableau_blocs)[izone].numero_secteur_base1_en_2eme ==0)
										(*tableau_blocs)[izone].numero_secteur_base1_en_2eme=
											numeros_secteurs_base1[izone];
								}
							}
						}
					}
				}
				valrenvoi=true;
			}
		}

	}
	return valrenvoi;
}
// ====================================
bool Classe_Piste::LitSecteursPisteBrute( // Lit la piste actuellement s�lectionn�e.
		class Classe_Disquette* classe_disquette, // classe appelante.
	Infos_Secteurs_Piste_brute_16ko* Resultat)
{
	if (Resultat==NULL)
	{
		return false;    
	}

	FD_READ_WRITE_PARAMS rwp =
		{ FD_OPTION_MFM,
		classe_disquette->infos_en_direct.Face_Selectionnee,
		classe_disquette->infos_en_direct.Piste_Selectionnee,
		classe_disquette->infos_en_direct.Face_Selectionnee,
		1,//start_
		7,//size_ :  127<<7 = 16384 octets.
		255,// eot_, // ou 1 ?
		1,  // gap
		0xff };  // datalen
	DWORD dwRet;
	bool OK=
		DeviceIoControl(
			classe_disquette->hDevice, IOCTL_FDCMD_READ_TRACK, &rwp, sizeof(rwp),
			&Resultat->Contenu_Piste_codee,//		 pv_,
			16384,//uLength_,
			&dwRet, NULL);
	if (OK)
	{
		OK &= DecodeTrack(Resultat);
	}


	return OK;
}
// ====================================
// Checksum a data block
WORD CrcBlock (const void* pcv_, size_t uLen_, WORD wCRC_=0xffff)
{
    static WORD awCRC[256];

    // Build the table if not already built
    if (!awCRC[1])
    {
        for (int i = 0 ; i < 256 ; i++)
        {
            WORD w = i << 8;

            // 8 shifts, for each bit in the update byte
            for (int j = 0 ; j < 8 ; j++)
                w = (w << 1) ^ ((w & 0x8000) ? 0x1021 : 0);

            awCRC[i] = w;
        }
    }

    // Update the CRC with each byte in the block
    const BYTE* pb = reinterpret_cast<const BYTE*>(pcv_);
    while (uLen_--)
        wCRC_ = (wCRC_ << 8) ^ awCRC[((wCRC_ >> 8) ^ *pb++) & 0xff];

    return wCRC_;
}
// ====================================
// Decode a bit-shifted run of bytes
void DecodeRun (BYTE *pb_, int nShift_, BYTE* pbOut_, int nLen_)
{
	for ( ; nLen_-- ; pb_++)
		*pbOut_++ = (pb_[0] << nShift_) | (pb_[1] >> (8-nShift_));
}
// ====================================
bool Classe_Piste::DecodeTrack (Infos_Secteurs_Piste_brute_16ko* Resultat)
{
	// Ici, on a la piste de 16 ko charg�e, et on doit renseigner la structure.
	const int nLen_=16384;
	BYTE* pb_= &Resultat->Contenu_Piste_codee[0];
	bool OK=false;

	int nPiste = 0, nFace = 0, nTaille_bits = 2;
	unsigned	index_mem_libre=0;

	int nShift, nHeader = 0;
	int nSector = 0, nSize = 512; // taille de secteur pas d�faut, au cas o� on ne trouve pas d'ID de secteur.

	for (int nPos = 0 ; nPos < nLen_-10 ; nPos++)
	{
		// Check all 8 shift positions for the start of an address mark
		for (nShift = 0 ; nShift < 8 ; nShift++)
		{
			BYTE b = (pb_[nPos] << nShift) | (pb_[nPos+1] >> (8-nShift));
			if (b == 0xa1)
				break;
		}

		// Move to next byte?
		if (nShift >= 8)
			continue;

		// Decode enough for an ID header (A1 A1 A1 idam cyl head sector size crchigh crclow)
		BYTE ab[10];
		DecodeRun(pb_+nPos, nShift, ab, sizeof(ab));

		// Check for start of address mark
		if (ab[0] == 0xa1 && ab[1] == 0xa1 && ab[2] == 0xa1)
		{
			if (ab[3] == 0xfe)	// id address mark
			{
				WORD wCrc = CrcBlock(ab, sizeof(ab)-2), wDiskCrc = (ab[8]<<8)|ab[9];

				if (wDiskCrc==wCrc)
				{
					nPiste = ab[4];
					nFace = ab[5];
					nSector = ab[6];
					nTaille_bits = ab[7]; // 0=128; 1=256; 2=512; etc..
					nSize = 128 << (ab[7] & 3);
					nHeader = nPos;
				}
			}
			else if (ab[3] == 0xfb || ab[3] == 0xf8)	// normal/deleted data address marks
			{
				if (nPos+4+nSize+2 < nLen_)	// enough data available?
				{
					BYTE ab[4+4096+2];
					DecodeRun(pb_+nPos, nShift, ab, 4+nSize+2);
					WORD wCrc = CrcBlock(ab, 4+nSize), wDiskCrc = (ab[4+nSize]<<8)|ab[4+nSize+1];

					// On a trouv� un secteur valide et v�rifi�.
					if (wDiskCrc==wCrc)
					{
						// If the data is too far from the last header, it lacks a header
						const bool idtrouve=(nPos-nHeader <= 50);
						
						{
							const unsigned isect=Resultat->Nb_Secteurs_trouves;

							//Resultat->Infos_Secteur[isect].Donnees_bien_lues=true;
							Resultat->Infos_Secteur[isect].Index_Dans_Contenu_Piste_Codee
              	= nPos;
							Resultat->Infos_Secteur[isect].Index_Dans_Contenu_Piste_Decodee
								= index_mem_libre;
							Resultat->Infos_Secteur[isect].ID_trouve_directement=idtrouve;
							Resultat->Infos_Secteur[isect].Secteur_identifie=idtrouve;
							if (idtrouve)
							{
								Resultat->Infos_Secteur[isect].ID_Piste=nPiste;
								Resultat->Infos_Secteur[isect].ID_Face=nFace;
								Resultat->Infos_Secteur[isect].ID_Secteur_base1=nSector;
								Resultat->Infos_Secteur[isect].ID_Taille=nTaille_bits;
							}
							else
								asm nop
							memcpy(
								&Resultat->Contenu_Piste_decodee[index_mem_libre],
								&ab[4],
								nSize);
							index_mem_libre += nSize;
							Resultat->Nb_Secteurs_trouves++;
							OK=true;
						}
					}

				}
			}
			else
				asm nop
		}
	}

	return OK;
}
// ====================================
infopiste*	Classe_Piste::CP_Analyse_Temps_Secteurs( // Analyse la piste, et fourni une carte temporelle des secteurs.
	class Classe_Disquette* classe_disquette, // classe appelante.
	unsigned piste,
	unsigned face)
{
	static infopiste infos_renvoyees;
	//
	memset(&infos_renvoyees,0,sizeof(infos_renvoyees));
	infos_renvoyees.OperationReussie=false; // par d�faut.
	if ( ! classe_disquette->fdrawcmd_sys_installe)
		return &infos_renvoyees;


	bool OK=false;

	DWORD dwRet;
	{
		// Avec ce pilote (fdrawcmd.sys), on est forc� d'utiliser une autre m�thode.
		FD_SEEK_PARAMS seekp;

		// details of seek location
		seekp.cyl = piste;
		seekp.head = face;

		OK = CP_selectionne_piste_et_face(classe_disquette,Piste_base0,Face_base0);
		if (OK)
		{
			classe_disquette->infos_en_direct.Piste_Selectionnee=piste; // La t�te est actuellement plac�e ici.
			classe_disquette->infos_en_direct.Face_Selectionnee=face; // Cette face est actuellement selectionn�e.
		}

	}

	// set up scan parameters
	FD_SCAN_PARAMS sp;
	sp.flags = FD_OPTION_MFM;
	sp.head = face;


	if (OK)
	{				// Lit le descriptif des secteurs, dans l'ordre, avec leur temps.
		static struct FD_TIMED_SCAN_RESULT_32 tsr;
		infos_renvoyees.fdrawcmd_Timed_Scan_Result=&tsr;

		// seek and scan track
		OK &= DeviceIoControl(classe_disquette->hDevice, IOCTL_FD_TIMED_SCAN_TRACK, &sp, sizeof(sp), &tsr, sizeof(tsr), &dwRet, NULL);
	}

	infos_renvoyees.OperationReussie=OK;
	return &infos_renvoyees;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

