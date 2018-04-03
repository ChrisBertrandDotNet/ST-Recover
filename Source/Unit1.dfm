object Form1: TForm1
  Left = 0
  Top = 0
  Caption = 'ST Recover'
  ClientHeight = 434
  ClientWidth = 686
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object LabelInformation: TLabel
    Left = 278
    Top = 375
    Width = 56
    Height = 13
    Caption = 'Information'
  end
  object Label1: TLabel
    Left = 8
    Top = 375
    Width = 55
    Height = 13
    Caption = 'Whole disk:'
  end
  object LabelTempsEcoule: TLabel
    Left = 612
    Top = 374
    Width = 62
    Height = 13
    Caption = 'Ellapsed time'
  end
  object ButtonReadDisk: TButton
    Left = 48
    Top = 7
    Width = 169
    Height = 25
    Caption = 'Read disk and save image file'
    TabOrder = 0
    OnClick = ButtonReadDiskClick
  end
  object ButtonAnalyse: TButton
    Left = 222
    Top = 7
    Width = 122
    Height = 25
    Caption = 'Analyse surface'
    TabOrder = 1
    OnClick = ButtonAnalyseClick
  end
  object BitBtnAnnule: TBitBtn
    Left = 544
    Top = 7
    Width = 136
    Height = 18
    Enabled = False
    TabOrder = 2
    OnClick = BitBtnAnnuleClick
    Kind = bkAbort
  end
  object PageControlResultats: TPageControl
    Left = 0
    Top = 37
    Width = 686
    Height = 331
    ActivePage = TabSheetTablesFaces
    TabOrder = 3
    object TabSheetTablesFaces: TTabSheet
      Caption = 'Tables'
      object DrawGridSecteursFaceA: TDrawGrid
        Left = 2
        Top = 2
        Width = 676
        Height = 148
        ColCount = 85
        DefaultColWidth = 9
        DefaultRowHeight = 14
        DefaultDrawing = False
        FixedCols = 0
        RowCount = 12
        FixedRows = 0
        ParentShowHint = False
        ShowHint = True
        TabOrder = 0
        OnDrawCell = DrawGridSecteursFaceADrawCell
        OnMouseMove = DrawGridSecteursFaceAMouseMove
      end
      object DrawGridSecteursFaceB: TDrawGrid
        Left = 2
        Top = 154
        Width = 676
        Height = 150
        ColCount = 85
        DefaultColWidth = 9
        DefaultRowHeight = 14
        DefaultDrawing = False
        FixedCols = 0
        RowCount = 12
        FixedRows = 0
        ParentShowHint = False
        ShowHint = True
        TabOrder = 1
        OnDrawCell = DrawGridSecteursFaceADrawCell
        OnMouseMove = DrawGridSecteursFaceAMouseMove
      end
    end
    object TabSheetFaces: TTabSheet
      Caption = 'Surface'
      ImageIndex = 1
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
      object ImageFace0: TImage
        Left = 19
        Top = 0
        Width = 305
        Height = 306
      end
      object ImageFace1: TImage
        Left = 353
        Top = 0
        Width = 306
        Height = 306
      end
    end
  end
  object ComboBoxTempsMaxi: TComboBox
    Left = 67
    Top = 373
    Width = 191
    Height = 21
    Style = csDropDownList
    ItemHeight = 0
    ItemIndex = 2
    TabOrder = 4
    Text = 'Giveup after 30 mn (Recommended)'
    Items.Strings = (
      'Giveup after 5 mn'
      'Giveup after 15 mn'
      'Giveup after 30 mn (Recommended)'
      'Giveup after 60 mn'
      'Giveup after 2 h'
      'Giveup after 5 h'
      'Never giveup (well, after 20 days)')
  end
  object ComboBoxDisque: TComboBox
    Left = 12
    Top = 7
    Width = 32
    Height = 21
    Style = csDropDownList
    ItemHeight = 0
    ItemIndex = 0
    TabOrder = 5
    Text = 'A:'
    Items.Strings = (
      'A:'
      'B:')
  end
  object MemoLOG: TMemo
    Left = 7
    Top = 392
    Width = 673
    Height = 37
    Hint = 'LOG'
    Lines.Strings = (
      'LOG:')
    ScrollBars = ssVertical
    TabOrder = 6
  end
  object CheckBoxSauveInfosPistesBrutes: TCheckBox
    Left = 355
    Top = 19
    Width = 185
    Height = 13
    Caption = 'Save track information (debug)'
    TabOrder = 7
  end
  object SaveDialogImageDisque: TSaveDialog
    DefaultExt = 'ST'
    FileName = 'Disk_.ST'
    Filter = 'ST disk image|*.ST'
    Options = [ofOverwritePrompt, ofHideReadOnly, ofPathMustExist, ofEnableSizing]
    Title = 'Save the disk image .ST'
    Left = 648
    Top = 24
  end
end
