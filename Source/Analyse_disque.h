//---------------------------------------------------------------------------
// Copyright Christophe Bertrand.
// This file is licensed under the Microsoft Reciprocal License (Ms-RL). You can find a copy of this license in the file "Ms-RL License.htm", in the root directory of the distribution archive.
//---------------------------------------------------------------------------



#ifndef Analyse_disqueH
#define Analyse_disqueH
//---------------------------------------------------------------------------
#include <Classes.hpp>


#include "Classe_Disquette.h"
#include "Constantes.h"

//---------------------------------------------------------------------------
class Analyse_Disque : public TThread
{
  typedef struct tagTHREADNAME_INFO
  {
    DWORD dwType;     // doit être 0x1000
    LPCSTR szName;    // pointeur sur le nom (dans l'espace d'adresse de l'utilisateur)
    DWORD dwThreadID; // ID de thread (-1=thread de l'appelant)
    DWORD dwFlags;    // réservé pour une future utilisation, doit être zéro
  } THREADNAME_INFO;
private:
	void SetName();
	void Analyse_Disque::metajour();
protected:
	void __fastcall Execute();
public:

	FD_TIMED_SCAN_RESULT_32	Tab_analyse_pistes[NB_MAX_PISTES][2];// maxi 85 pistes et 2 faces.
	Classe_Disquette* classe_disque;
	bool		Thread_en_route;

	__fastcall Analyse_Disque(bool CreateSuspended);
};
//---------------------------------------------------------------------------
#endif
