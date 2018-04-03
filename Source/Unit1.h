//---------------------------------------------------------------------------
// Copyright Christophe Bertrand.
// This file is licensed under the Microsoft Reciprocal License (Ms-RL). You can find a copy of this license in the file "Ms-RL License.htm", in the root directory of the distribution archive.
//---------------------------------------------------------------------------



#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Grids.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Dialogs.hpp>



//---------------------------------------------------------------------------

// Message personnalisé pour ordonner la mise à jour du graphique.
#define WM_recover_maj_piste_FormAnalyse (WM_APP+1)

/*		ComboBoxTempsMaxi   : donne un temps maxi pour lire la disquette.

0		Giveup after 5 mn
1		Giveup after 15 mn
2		Giveup after 30 mn (Recommended)
3		Giveup after 60 mn
4		Giveup after 2 h
5		Giveup after 5 h
6		Never giveup (well, after 20 days)
*/
const int Temps_ComboBoxTempsMaxi_en_ms[7]={ 300000,900000,1800000,3600000,7200000,18000000,1728000000 };


//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// Composants gérés par l'EDI
	TButton *ButtonReadDisk;
	TLabel *LabelInformation;
	TButton *ButtonAnalyse;
	TBitBtn *BitBtnAnnule;
	TPageControl *PageControlResultats;
	TTabSheet *TabSheetTablesFaces;
	TDrawGrid *DrawGridSecteursFaceA;
	TDrawGrid *DrawGridSecteursFaceB;
	TTabSheet *TabSheetFaces;
	TImage *ImageFace0;
	TImage *ImageFace1;
	TComboBox *ComboBoxTempsMaxi;
	TSaveDialog *SaveDialogImageDisque;
	TComboBox *ComboBoxDisque;
	TLabel *Label1;
	TMemo *MemoLOG;
	TLabel *LabelTempsEcoule;
	TCheckBox *CheckBoxSauveInfosPistesBrutes;
	void __fastcall ButtonReadDiskClick(TObject *Sender);
	void __fastcall DrawGridSecteursFaceADrawCell(TObject *Sender, int ACol,
          int ARow, TRect &Rect, TGridDrawState State);
	void __fastcall ButtonAnalyseClick(TObject *Sender);
	void __fastcall BitBtnAnnuleClick(TObject *Sender);
	void __fastcall DrawGridSecteursFaceAMouseMove(TObject *Sender,
          TShiftState Shift, int X, int Y);
protected:
	void __fastcall On_recover_maj_piste_FormAnalyse(TMessage &Message);
	BEGIN_MESSAGE_MAP  // Voir http://www.programmez.com/tutoriels.php?tutoriel=38&titre=Les-messages-Windows-personnalises-avec-C++-Builder
		VCL_MESSAGE_HANDLER(WM_recover_maj_piste_FormAnalyse, TMessage, On_recover_maj_piste_FormAnalyse)
	END_MESSAGE_MAP(TForm)
private:	// Déclarations de l'utilisateur
public:		// Déclarations de l'utilisateur

	bool PleaseCancelCurrentOperation;

	__fastcall TForm1(TComponent* Owner);
	bool	__fastcall AnalyseDisquette(void);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
