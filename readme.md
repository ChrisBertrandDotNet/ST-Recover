# ST Recover 

![](Manual/images/cercle_surface_10_secteurs_sur4.png?raw=true)  
## ST Recover  

  
  
![](Manual/images/interface_secteurs_sur2.png?raw=true)  ![](Manual/images/interface_surface_sur2.png?raw=true)  

  
## Project Description
  
ST Recover can read Atari ST floppy disks on a PC under Windows, including special formats as 800 or 900 KB and damaged or desynchronized disks, and produces standard .ST disk image files. Then the image files can be read in ST emulators as WinSTon or [Steem](http://steem.atari.st/index.htm).  
  
![](Manual/images/schema_transfert.png?raw=true)  
ST disk  ->  PC floppy drive  ->  .ST disk image file  
  
  
## Required configuration  

*   A PC with a floppy disk drive.  
    Internal is better, but external can be used with limitations.
*   Windows.  
    Should be compatible on Windows 8,7,Vista,XP,2000,ME,98,95 and later.  
    Tested on Vista. Windows 10 is not guaranted.
    
  
## The particularities  
  
1.  Normally, Windows cannot read special formats (as 800 or 900 KB) because of limitations of its floppy driver.  
Unfortunately, these formats are very common in the world of the ST.  
ST Recover takes advantage of a [free driver](http://simonowen.com/fdrawcmd/) that can read these special formats.  
  
2.  An effort has been done to recover desynchronized or damaged disks.  
  
3.  You can see the geography of sectors on your disk:  
![](Manual/images/cercle_surface_10_secteurs.png?raw=true) This is the surface of one face of a real 800 Kb disk. The black line on the right is the virtual synchronization mark.  
  
  
  
## On your real PC, a not-USB floppy disk drive is needed for special formats  

### External (USB) floppy drive

![](Manual/images/lecteur_disquettes_externe.png?raw=true)  

Limited to:  

*   360 KB
*   720 KB  

With a USB disk drive, ST Recover cannot read special formats, because of the interface limitations.  
But a USB drive can read ordinary formats as 360 or 720 Kb.

### Internal floppy drive
    
![](Manual/images/lecteur_disquettes_interne.png?raw=true)  

Allows all formats:  

*   360 KB
*   720 KB  
*   400 KB
*   800 KB  
*   450 KB
*   900 KB

Etc..  

  
## Installation  

There are 4 steps:  

1. If you have an internal floppy drive, install [this external free software](http://simonowen.com/fdrawcmd/#download).
Then run *FdInstall.exe*.
2. Download the [Zip file](https://github.com/ChrisBertrandDotNet/ST-Recover/releases/download/Windows/ST_Recover_1.0_90EB9F5DB9540A0A25D0442E25F100DCC6B2A8FB3D889594BC38195AE2502AF2.zip) of ST Recover.
3. Extract it.
4. Run *ST_Recover.exe*.  
    There is no installer, it is portable.
     
Use  

1. Run ST Recover.
2. Insert an ST disk in the PC drive.
3. If necessary, select the right drive (A or B).      
4. Click on "Read disk and save image file".
5. Select the location and the name of the image file to be created.
6. Wait.  
    

  
## Surface analysis  
  
If your disk seems to be damaged, or just for curiosity, click on "Analyze surface" to see a geographic analysis of the sectors on the disk.  
![](Manual/images/cercle_surface_10_secteurs.png?raw=true)  
Each "split circle" represents a Track (the color is alternated from one track to the next one), and each part of this circle represents a Sector, which is a packet of 512 bytes (in general).  
The line on the right represent the location of the virtual synchronization mark on the disk. Sectors are read counterclockwise from the mark.  
  
This representation is computed from a timing analysis during sectors reading. So it represents the real disposition of the sectors on the disk (except for the diameter of the inner circles, which is relatively bigger in reality).  
  
It is interesting to notice that the outer sectors occupy more physical space than the inner sectors (in reality they are about twice bigger). That explains why the first sectors (the track 0 is the outer track) are considered more secure than the last sectors (the inner track is the last one) : more magnetic surface for the same amount of data.  

On modern hard disks, the surface of the sectors is (almost) constant, that allows to gain 50% sectors more but needs a (bit) more complex processing. On cdroms, the surface/sector is constant but the rotation speed change. Anyway many technicals have changed since the old time of the floppy disks, MFM hopefully disappeared, we have a constant surface/sector, there are more than two levels per information, etc..  
  
  
## FAQ

*   The software says "*No disk in drive*" but there is a disk !

*   Maybe the very first sector, the boot sector, is not read correctly.  
    Try again to read the disk, it should work after a few tries.  
    

*   What mean the colors in the Tables of sectors ?
    *   White: not analyzed for now.
    *   Green: read (with ease).
    *   Dark blue: read (with more analysis).
    *   Red: cannot be read at all (maybe physically damaged), even after several tries.
*   What if a sector cannot be read ?
    *   ST Recover will replace its data by a repetitive sentence, in order to help you to detect it with ease.  
        It should be "*======== SORRY, THIS SECTOR CANNOT BE READ FROM FLOPPY DISK BY ST RECOVER. ========*", or something close.  
        In the Tables of sectors, this sector will be marked as red.
*   In the Surface graphs, the "circles" are not closed. Is there missing something ?
    
    *   Usually, in that case one sector is not easily read. That is very common with the disks formatted using some ST programs (as _Fastcopy Pro_, for example), with more than 9 sectors/track. The first sector of each track is not seen by the floppy controller in ordinary conditions.  
        Fortunately, in most case ST Recover is able to read these sectors, at the price of a slow analysis. These sectors are marked as dark blue in the Tables of sectors.
    
*   Why some sectors are marked as dark blue in the Tables of sectors, and the analysis is very slow ?
    *   See above for an explanation. These sectors are correctly read, although slowly.

  
## Compilation of the source code  
  
The code was edited and compiled with [Borland Turbo C++ 2006 Explorer](http://www.turboexplorer.com/), a free RAD.  
It is supposed to be compilable with later versions of Borland ([Embarcadero](http://www.embarcadero.com)) C++ Builder. I did not try them since I don't have any of these RADs.  
Turbo C++ 2006 Explorer was free but it seems not to be distributed anymore. It needed a registration and the installation of several software as dot.net 1 and JSharp.  
By the way, my source code is commented in french.  
  
  
## License  
  
The license for the source code and the binaries is the [Ms-RL](https://opensource.org/licenses/MS-RL), which globally allows you to reproduce the code, modify it, distribute it, but you have to publish the source code of the modifications.  
Additionally, this software is patent-free.  
  
  
## Slow disks  
  
Sometimes, a disk takes time to be read.  
The possible reasons can be:  

*   The disk is desynchronized.
*   The disk is partially physically damaged.  
    In that case, sectors will be colored in red.  
    

  
## Desynchronized disk  
  
This kind of disk will take more time to be read, and the sector tables will display blue rectangles:  
![](Manual/images/secteurs_desynchronises.png?raw=true)  
  
Blue rectangles indicate particular, but well-read, sectors. In fact, here these sectors only need more time to be read, but they are read correctly (otherwise, the color would be red).  
  
To understand what is desynchronization, run a surface analysis:  
![](Manual/images/surface_avec_secteur_marque_256c.png?raw=true)  
As you can see, one sector per track is not detected.The synchronization mark (the green line) passes through where the sectors are supposed to be (the red triangle).  
The explanation is: the drive's head passes ahead the track from the line, but it is too late to see the beginning of the sector, so the first sector is not detected by the surface test.  
But trying to read the first sector is possible on the next turn, it just takes more time.  
  
More details: I had to implement a particular analysis of the track, in order to be able to extract the (blue) sector information. Fortunately, the special driver can extract a whole track ! That is a question of [MFM](https://en.wikipedia.org/wiki/Modified_Frequency_Modulation) decoding, with partially desynchronised (6250) bytes per track and so on.  
By the way, the decoding & recover method is all about MFM bit desynchronization, nothing to do with the disk synchronization mark:  
The disk desynchronized mark is the origin of the problem, due to a bad setting in the drive which originally formatted the disk, and the MFM decoding and resynchronization if a method I use to recover the sector.  
  
  
If you followed the explanation, you should have noticed that the problem is with the first sector of each track. So why in the table of sectors the blue rectangles are not all in the left row ? Because the sectors were not numbered normally during formatting, they were shifted in order to allow the drive controller (the processor that manages the drive) more time to move the head to the next track (it takes time and the disk goes on turning) and to read as early as possible the next sector.  
Sorry if I am not clear enough, you should find information on internet about how speeding up floppy disk accesses through [sector interleaving](https://en.wikipedia.org/wiki/Interleaving_(disk_storage)).  
  
  
## Not tested  
  
There are formats I did not test:  

*   Sector length different from 512 bytes (ex: 128, 256 or 1024).  
    As far as I remember, I did not write much logic to take those lengths into account (it was not my goal).  
    
*   Protected disks where physical format is different from the logical format (as indicated in the boot sector).  
    For example some sectors can be numbered not from 1 but from 40.
*   Protected disks where sectors are not well-formed.  
    For example they can be cut in the middle just to fool the disk copy programs.
*   900 KB disks. I need to format a new 900 KB disk on my old ST then try to read it with my rather old desktop PC.  
    

The reason I made this little tool  is to be able to recover my own old work that was ST disks. Not to analyze protected or abnormal disks.
  
  
## Legal  
  
Text and images of this document are copyright by Christophe Bertrand under the [CC-BY-SA license](https://creativecommons.org/licenses/by-sa/3.0/), except:  

*   The Atari ST image is under the CC-BY-2.5 — Attribution license by Bill Bertram. See [here](https://en.wikipedia.org/wiki/File:Atari_1040STf.jpg).
*   The Crystal floppy, hard-disk and directory icons are under the LGPL, and were from the [Crystal Project](http://www.everaldo.com/crystal/).
*   The desktop PC image is public domain and was found [here](https://commons.wikimedia.org/wiki/File:Desktop-PC.svg).