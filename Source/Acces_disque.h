//---------------------------------------------------------------------------
// Copyright Christophe Bertrand.
// This file is licensed under the Microsoft Reciprocal License (Ms-RL). You can find a copy of this license in the file "Ms-RL License.htm", in the root directory of the distribution archive.
//---------------------------------------------------------------------------


#ifndef Acces_disqueH
#define Acces_disqueH
//---------------------------------------------------------------------------
#include <Classes.hpp>

#include "Classe_Disquette.h"



//---------------------------------------------------------------------------
class Acces_Disque : public TThread
{
  typedef struct tagTHREADNAME_INFO
  {
    DWORD dwType;     // doit �tre 0x1000
    LPCSTR szName;    // pointeur sur le nom (dans l'espace d'adresse de l'utilisateur)
    DWORD dwThreadID; // ID de thread (-1=thread de l'appelant)
    DWORD dwFlags;    // r�serv� pour une future utilisation, doit �tre z�ro
  } THREADNAME_INFO;
private:

	RECT rect_invalide;
	TDrawGrid* grid;

	void SetName();
	void __fastcall MetAJourLAffichage();
protected:
	void __fastcall Execute();
public:

	Classe_Disquette* classe_disque;
	bool		Thread_en_route;


	__fastcall Acces_Disque(bool CreateSuspended);
};
//---------------------------------------------------------------------------
#endif
