
#include"../../emul/cpu.h"

#include"../data.h"
#include"../../emul/starflt1.h"

#include"../overlays/CULTURE.h"
#include"../overlays/FX.h"
#include"../overlays/ENDING.h"
#include"../overlays/DEPART.h"
#include"../overlays/CHKFLIGHT-OV.h"
#include"../overlays/FRACT-OV.h"
#include"../overlays/ICONP-OV.h"
#include"../overlays/SITE-OV.h"
#include"../overlays/HYPERMSG-OV.h"
#include"../overlays/BLT-OV.h"
#include"../overlays/MISC-OV.h"
#include"../overlays/BANK-OV.h"
#include"../overlays/ASSCREW-OV.h"
#include"../overlays/PERSONNEL-OV.h"
#include"../overlays/SHIPGRPH-OV.h"
#include"../overlays/CONFIG-OV.h"
#include"../overlays/TDEPOT-OV.h"
#include"../overlays/EDIT-OV.h"
#include"../overlays/PORTMENU-OV.h"
#include"../overlays/VITA-OV.h"
#include"../overlays/EYE-OV.h"
#include"../overlays/DESCRIBE.h"
#include"../overlays/TV-OV.h"
#include"../overlays/COMM-OV.h"
#include"../overlays/COMMSPEC-OV.h"
#include"../overlays/SEED-OV.h"
#include"../overlays/LISTICONS.h"
#include"../overlays/MOVE-OV.h"
#include"../overlays/ENGINEER.h"
#include"../overlays/DOCTOR.h"
#include"../overlays/ORBIT-OV.h"
#include"../overlays/CAPTAIN.h"
#include"../overlays/SCIENCE.h"
#include"../overlays/NAVIGATR.h"
#include"../overlays/SHIPBUTTONS.h"
#include"../overlays/MAP-OV.h"
#include"../overlays/HYPER-OV.h"
#include"../overlays/ANALYZE-OV.h"
#include"../overlays/LAUNCH-OV.h"
#include"../overlays/FLUX-EFFECT.h"
#include"../overlays/OP-OV.h"
#include"../overlays/TVITEMS.h"
#include"../overlays/BEHAV-OV.h"
#include"../overlays/HEAL-OV.h"
#include"../overlays/REPAIR-OV.h"
#include"../overlays/PLSET-OV.h"
#include"../overlays/MAPS-OV.h"
#include"../overlays/STORM-OV.h"
#include"../overlays/COMBAT-OV.h"
#include"../overlays/DAMAGE-OV.h"
#include"../overlays/LAND-OV.h"
#include"../overlays/MUSIC.h"
#include"../overlays/MODGAME.h"
#include"../overlays/STP-OV.h"
#include"../overlays/COMBAUX.h"
#include"../overlays/IT-OV.h"
#include"../overlays/BLDLISTS.h"
#include"../overlays/BARTER.h"
#include"../overlays/MARKET.h"
#include"../overlays/TVCON-OV.h"
#include"../overlays/CLOUD-OV.h"
#include"../overlays/JUMP.h"
#include"../overlays/SHIPCON.h"
#include"../overlays/DRONE.h"
#include"../overlays/WEAPONS.h"

// =================================
// =========== Dictionary ==========
// =================================
//          (!SET)  codep:0x0138 wordp:0x0138 size:0x001d C-string:'_ro__ex_SET_rc_'
//          (!OLD)  codep:0x0160 wordp:0x0160 size:0x001d C-string:'_ro__ex_OLD_rc_'
//          (TIME)  codep:0x1d29 wordp:0x0188 size:0x0068 C-string:'_ro_TIME_rc_'
//           SET0/  codep:0x01fa wordp:0x01fa size:0x001a C-string:'SET0_slash_'
//       (RESTORE)  codep:0x224c wordp:0x0222 size:0x0048 C-string:'_ro_RESTORE_rc_'
//      AUTO-CACHE  codep:0x224c wordp:0x0279 size:0x003a C-string:'AUTO_dash_CACHE'
//      AUTO-LIMIT  codep:0x224c wordp:0x02c2 size:0x0024 C-string:'AUTO_dash_LIMIT'
//     AUTO-SCREEN  codep:0x224c wordp:0x02f6 size:0x003a C-string:'AUTO_dash_SCREEN'
//            COLD  codep:0x224c wordp:0x0339 size:0x0008 C-string:'COLD'
//           W0343  codep:0x224c wordp:0x0343 size:0x007e C-string:'W0343'
//           W03C3  codep:0x224c wordp:0x03c3 size:0x0008 C-string:'W03C3'
//     SET-BUFFERS  codep:0x224c wordp:0x03db size:0x002a C-string:'SET_dash_BUFFERS'
//             SP0  codep:0x1792 wordp:0x040d size:0x0002 C-string:'SP0'
//              R0  codep:0x1792 wordp:0x0416 size:0x0002 C-string:'R0'
//              DP  codep:0x1792 wordp:0x041f size:0x0002 C-string:'DP'
//           FENCE  codep:0x1792 wordp:0x042b size:0x0002 C-string:'FENCE'
//          (INTO)  codep:0x1792 wordp:0x0438 size:0x0002 C-string:'_ro_INTO_rc_'
//        MSGFLAGS  codep:0x1792 wordp:0x0447 size:0x0002 C-string:'MSGFLAGS'
//        VOC-LINK  codep:0x1792 wordp:0x0456 size:0x0002 C-string:'VOC_dash_LINK'
//        USERSIZE  codep:0x1792 wordp:0x0465 size:0x0002 C-string:'USERSIZE'
//        USERNEXT  codep:0x1792 wordp:0x0474 size:0x0002 C-string:'USERNEXT'
//             >IN  codep:0x1792 wordp:0x047e size:0x0002 C-string:'_gt_IN'
//             TIB  codep:0x1792 wordp:0x0488 size:0x0002 C-string:'TIB'
//       VIDEOPORT  codep:0x1792 wordp:0x0498 size:0x0002 C-string:'VIDEOPORT'
//        VIDEOMEM  codep:0x1792 wordp:0x04a7 size:0x0002 C-string:'VIDEOMEM'
//       CURSORLOC  codep:0x1792 wordp:0x04b7 size:0x0002 C-string:'CURSORLOC'
//             ROW  codep:0x1792 wordp:0x04c1 size:0x0002 C-string:'ROW'
//             COL  codep:0x1792 wordp:0x04cb size:0x0002 C-string:'COL'
//          ROWMIN  codep:0x1792 wordp:0x04d8 size:0x0002 C-string:'ROWMIN'
//          COLMIN  codep:0x1792 wordp:0x04e5 size:0x0002 C-string:'COLMIN'
//          ROWMAX  codep:0x1792 wordp:0x04f2 size:0x0002 C-string:'ROWMAX'
//          COLMAX  codep:0x1792 wordp:0x04ff size:0x0002 C-string:'COLMAX'
//       ATTRIBUTE  codep:0x1792 wordp:0x050f size:0x0002 C-string:'ATTRIBUTE'
//         DOFFSET  codep:0x1792 wordp:0x051d size:0x0002 C-string:'DOFFSET'
//      SERIALPORT  codep:0x1792 wordp:0x052e size:0x0002 C-string:'SERIALPORT'
//    PARALLELPORT  codep:0x1792 wordp:0x0541 size:0x0002 C-string:'PARALLELPORT'
//            BASE  codep:0x1792 wordp:0x054c size:0x0002 C-string:'BASE'
//             BLK  codep:0x1792 wordp:0x0556 size:0x0002 C-string:'BLK'
//             DPL  codep:0x1792 wordp:0x0560 size:0x0002 C-string:'DPL'
//             FLD  codep:0x1792 wordp:0x056a size:0x0002 C-string:'FLD'
//             HLD  codep:0x1792 wordp:0x0574 size:0x0002 C-string:'HLD'
//              R#  codep:0x1792 wordp:0x057d size:0x0002 C-string:'R_n_'
//             SCR  codep:0x1792 wordp:0x0587 size:0x0002 C-string:'SCR'
//            SSCR  codep:0x1792 wordp:0x0592 size:0x0002 C-string:'SSCR'
//           STATE  codep:0x1792 wordp:0x059e size:0x0002 C-string:'STATE'
//       CONTEXT_1  codep:0x1792 wordp:0x05ac size:0x0002 C-string:'CONTEXT_1'
//             CSP  codep:0x1792 wordp:0x05b6 size:0x0002 C-string:'CSP'
//         CURRENT  codep:0x1792 wordp:0x05c4 size:0x0002 C-string:'CURRENT'
//          LFALEN  codep:0x1792 wordp:0x05d1 size:0x0002 C-string:'LFALEN'
//       LOWERCASE  codep:0x1792 wordp:0x05e1 size:0x0002 C-string:'LOWERCASE'
//     SEARCHORDER  codep:0x1792 wordp:0x05f3 size:0x0002 C-string:'SEARCHORDER'
//           WIDTH  codep:0x1792 wordp:0x05ff size:0x0002 C-string:'WIDTH'
//           -FIND  codep:0x17b7 wordp:0x060b size:0x0002 C-string:'_dash_FIND'
//       ?TERMINAL  codep:0x17b7 wordp:0x061b size:0x0002 C-string:'IsTERMINAL'
//           ABORT  codep:0x17b7 wordp:0x0627 size:0x0002 C-string:'ABORT'
//            BELL  codep:0x17b7 wordp:0x0632 size:0x0002 C-string:'BELL'
//              BS  codep:0x17b7 wordp:0x063b size:0x0002 C-string:'BS'
//              CR  codep:0x17b7 wordp:0x0644 size:0x0002 C-string:'CR'
//          CREATE  codep:0x17b7 wordp:0x0651 size:0x0002 C-string:'CREATE'
//      DISKERROR?  codep:0x17b7 wordp:0x0662 size:0x0002 C-string:'DISKERROR_ask_'
//            EMIT  codep:0x17b7 wordp:0x066d size:0x0002 C-string:'EMIT'
//          EXPECT  codep:0x17b7 wordp:0x067a size:0x0002 C-string:'EXPECT'
//       INTERPRET  codep:0x17b7 wordp:0x068a size:0x0002 C-string:'INTERPRET'
//          LINKS>  codep:0x17b7 wordp:0x0697 size:0x0002 C-string:'LINKS_gt_'
//          LOAD_1  codep:0x17b7 wordp:0x06a2 size:0x0002 C-string:'LOAD_1'
//           KEY_1  codep:0x17b7 wordp:0x06ac size:0x0002 C-string:'KEY_1'
//          NUMBER  codep:0x17b7 wordp:0x06b9 size:0x0002 C-string:'NUMBER'
//            PAGE  codep:0x17b7 wordp:0x06c4 size:0x0002 C-string:'PAGE'
//        POSITION  codep:0x17b7 wordp:0x06d3 size:0x0002 C-string:'POSITION'
//             R/W  codep:0x17b7 wordp:0x06dd size:0x0002 C-string:'R_slash_W'
//            TYPE  codep:0x17b7 wordp:0x06e8 size:0x0002 C-string:'TYPE'
//           WHERE  codep:0x17b7 wordp:0x06f4 size:0x0002 C-string:'WHERE'
//            WORD  codep:0x17b7 wordp:0x06ff size:0x0080 C-string:'WORD'
//        OPERATOR  codep:0x1d29 wordp:0x078c size:0x0084 C-string:'OPERATOR'
//           FORTH  codep:0x1ab5 wordp:0x081a size:0x000e C-string:'FORTH'
//          FREEZE  codep:0x224c wordp:0x0833 size:0x0022 C-string:'FREEZE'
//           BYE_1  codep:0x224c wordp:0x085d size:0x000a C-string:'BYE_1'
//          CHANGE  codep:0x224c wordp:0x0872 size:0x005f C-string:'CHANGE'
//         (SETUP)  codep:0x224c wordp:0x08dd size:0x005a C-string:'_ro_SETUP_rc_'
//           W0939  codep:0x224c wordp:0x0939 size:0x005a C-string:'W0939'
//              BL  codep:0x2214 wordp:0x099a size:0x0002 C-string:'BL'
//             C/L  codep:0x2214 wordp:0x09a4 size:0x0002 C-string:'C_slash_L'
//      INIT-FORTH  codep:0x2214 wordp:0x09b5 size:0x0002 C-string:'INIT_dash_FORTH'
//       INIT-USER  codep:0x2214 wordp:0x09c5 size:0x0002 C-string:'INIT_dash_USER'
//      INITIAL-DP  codep:0x2214 wordp:0x09d6 size:0x0002 C-string:'INITIAL_dash_DP'
//           LIMIT  codep:0x2214 wordp:0x09e2 size:0x0002 C-string:'LIMIT'
//          #CACHE  codep:0x1d29 wordp:0x09ef size:0x0002 C-string:'_n_CACHE'
//        #BUFFERS  codep:0x1d29 wordp:0x09fe size:0x0002 C-string:'_n_BUFFERS'
//          #SPACE  codep:0x1d29 wordp:0x0a0b size:0x0002 C-string:'_n_SPACE'
//       'RESTORE+  codep:0x1d29 wordp:0x0a1b size:0x0002 C-string:'_i_RESTORE_plus_'
//         'SETUP+  codep:0x1d29 wordp:0x0a29 size:0x0002 C-string:'_i_SETUP_plus_'
//          @,0*1;  codep:0x1d29 wordp:0x0a36 size:0x0002 C-string:'Get_co_0_star_1_sc_'
//          >BREAK  codep:0x1d29 wordp:0x0a43 size:0x0002 C-string:'_gt_BREAK'
//       BOOT-HOOK  codep:0x1d29 wordp:0x0a53 size:0x0002 C-string:'BOOT_dash_HOOK'
//       BOOT-LOAD  codep:0x1d29 wordp:0x0a63 size:0x0002 C-string:'BOOT_dash_LOAD'
//    BUFFER-BEGIN  codep:0x1d29 wordp:0x0a76 size:0x0002 C-string:'BUFFER_dash_BEGIN'
//     BUFFER-HEAD  codep:0x1d29 wordp:0x0a88 size:0x0002 C-string:'BUFFER_dash_HEAD'
//     CACHE-BEGIN  codep:0x1d29 wordp:0x0a9a size:0x0002 C-string:'CACHE_dash_BEGIN'
//      CACHE-HEAD  codep:0x1d29 wordp:0x0aab size:0x0002 C-string:'CACHE_dash_HEAD'
//          'FORTH  codep:0x224c wordp:0x0ab8 size:0x0012 C-string:'_i_FORTH'
//              ME  codep:0x0ad1 wordp:0x0ad1 size:0x0006 C-string:'ME'
//           W0AD9  codep:0x224c wordp:0x0ad9 size:0x0037 C-string:'W0AD9'
//           W0B12  codep:0x224c wordp:0x0b12 size:0x001c C-string:'W0B12'
//           W0B30  codep:0x224c wordp:0x0b30 size:0x002b C-string:'W0B30'
//           W0B5D  codep:0x224c wordp:0x0b5d size:0x0013 C-string:'W0B5D'
//         ?ENOUGH  codep:0x224c wordp:0x0b7c size:0x002e C-string:'IsENOUGH'
//               @  codep:0x0bb0 wordp:0x0bb0 size:0x0008 C-string:'Get'
//              @L  codep:0x0bbf wordp:0x0bbf size:0x001c C-string:'GetL'
//               !  codep:0x0be1 wordp:0x0be1 size:0x0009 C-string:'Store'
//              !L  codep:0x0bf1 wordp:0x0bf1 size:0x001c C-string:'StoreL'
//           (CS?)  codep:0x0c17 wordp:0x0c17 size:0x0006 C-string:'_ro_CS_ask__rc_'
//            2!_1  codep:0x0c24 wordp:0x0c24 size:0x000f C-string:'_2_ex__1'
//              2@  codep:0x0c3a wordp:0x0c3a size:0x000f C-string:'_2_at_'
//           BLANK  codep:0x224c wordp:0x0c53 size:0x0006 C-string:'BLANK'
//            C!_1  codep:0x0c60 wordp:0x0c60 size:0x0009 C-string:'C_ex__1'
//             C!L  codep:0x0c71 wordp:0x0c71 size:0x001c C-string:'C_ex_L'
//              C@  codep:0x0c94 wordp:0x0c94 size:0x000b C-string:'C_at_'
//             C@L  codep:0x0ca7 wordp:0x0ca7 size:0x001e C-string:'C_at_L'
//           COUNT  codep:0x224c wordp:0x0ccf size:0x000a C-string:'COUNT'
//          CMOVE>  codep:0x0ce4 wordp:0x0ce4 size:0x0022 C-string:'CMOVE_gt_'
//         CMOVE_1  codep:0x0d10 wordp:0x0d10 size:0x001c C-string:'CMOVE_1'
//          FILL_1  codep:0x0d35 wordp:0x0d35 size:0x001a C-string:'FILL_1'
//              P!  codep:0x0d56 wordp:0x0d56 size:0x0008 C-string:'P_ex_'
//              P@  codep:0x0d65 wordp:0x0d65 size:0x000a C-string:'P_at_'
//        SEG>ADDR  codep:0x0d7c wordp:0x0d7c size:0x0013 C-string:'SEG_gt_ADDR'
//        ADDR>SEG  codep:0x0d9c wordp:0x0d9c size:0x0013 C-string:'ADDR_gt_SEG'
//              >R  codep:0x0db6 wordp:0x0db6 size:0x000b C-string:'_gt_R'
//            ?DUP  codep:0x0dca wordp:0x0dca size:0x000c C-string:'IsDUP'
//           2DROP  codep:0x0de0 wordp:0x0de0 size:0x0009 C-string:'_2DROP'
//            2DUP  codep:0x0df2 wordp:0x0df2 size:0x000c C-string:'_2DUP'
//           2SWAP  codep:0x0e08 wordp:0x0e08 size:0x000d C-string:'_2SWAP'
//           DEPTH  codep:0x224c wordp:0x0e1f size:0x000c C-string:'DEPTH'
//            DROP  codep:0x0e34 wordp:0x0e34 size:0x0007 C-string:'DROP'
//             DUP  codep:0x0e43 wordp:0x0e43 size:0x0009 C-string:'DUP'
//               I  codep:0x0e52 wordp:0x0e52 size:0x0009 C-string:'I'
//              I'  codep:0x0e62 wordp:0x0e62 size:0x0008 C-string:'I_i_'
//               J  codep:0x0e70 wordp:0x0e70 size:0x0008 C-string:'J'
//            OVER  codep:0x0e81 wordp:0x0e81 size:0x000a C-string:'OVER'
//              R>  codep:0x0e92 wordp:0x0e92 size:0x000b C-string:'R_gt_'
//              R@  codep:0x0ea4 wordp:0x0ea4 size:0x0009 C-string:'R_at_'
//             ROT  codep:0x0eb5 wordp:0x0eb5 size:0x000b C-string:'ROT'
//             RP!  codep:0x0ec8 wordp:0x0ec8 size:0x0008 C-string:'RP_ex_'
//             RP@  codep:0x0ed8 wordp:0x0ed8 size:0x0006 C-string:'RP_at_'
//              S0  codep:0x224c wordp:0x0ee5 size:0x0006 C-string:'S0'
//            SWAP  codep:0x0ef4 wordp:0x0ef4 size:0x0009 C-string:'SWAP'
//             SP!  codep:0x0f05 wordp:0x0f05 size:0x0007 C-string:'SP_ex_'
//             SP@  codep:0x0f14 wordp:0x0f14 size:0x0008 C-string:'SP_at_'
//               0  codep:0x0f22 wordp:0x0f22 size:0x0008 C-string:'_0'
//               1  codep:0x0f30 wordp:0x0f30 size:0x0009 C-string:'_1'
//               2  codep:0x0f3f wordp:0x0f3f size:0x0009 C-string:'_2'
//               /  codep:0x0f4e wordp:0x0f4e size:0x000b C-string:'_slash_'
//            /MOD  codep:0x0f62 wordp:0x0f62 size:0x000c C-string:'_slash_MOD'
//               +  codep:0x0f74 wordp:0x0f74 size:0x000a C-string:'_plus_'
//              +!  codep:0x0f85 wordp:0x0f85 size:0x0009 C-string:'_plus__ex_'
//               -  codep:0x0f94 wordp:0x0f94 size:0x000a C-string:'_dash_'
//              +-  codep:0x224c wordp:0x0fa5 size:0x000a C-string:'_plus__dash_'
//               *  codep:0x0fb5 wordp:0x0fb5 size:0x000a C-string:'_star_'
//              */  codep:0x224c wordp:0x0fc6 size:0x0008 C-string:'_star__slash_'
//           */MOD  codep:0x224c wordp:0x0fd8 size:0x000a C-string:'_star__slash_MOD'
//              1+  codep:0x0fe9 wordp:0x0fe9 size:0x0008 C-string:'_1_plus_'
//              1-  codep:0x0ff8 wordp:0x0ff8 size:0x0008 C-string:'_1_dash_'
//              2*  codep:0x1007 wordp:0x1007 size:0x0009 C-string:'_2_star_'
//              2+  codep:0x1017 wordp:0x1017 size:0x0009 C-string:'_2_plus_'
//              2-  codep:0x1027 wordp:0x1027 size:0x0009 C-string:'_2_dash_'
//              2/  codep:0x1037 wordp:0x1037 size:0x0009 C-string:'_2_slash_'
//              4*  codep:0x1047 wordp:0x1047 size:0x000b C-string:'_4_star_'
//             ABS  codep:0x224c wordp:0x105a size:0x0006 C-string:'ABS'
//              D+  codep:0x1067 wordp:0x1067 size:0x000f C-string:'D_plus_'
//             D+-  codep:0x224c wordp:0x107e size:0x000a C-string:'D_plus__dash_'
//         DECIMAL  codep:0x224c wordp:0x1094 size:0x000a C-string:'DECIMAL'
//            DABS  codep:0x224c wordp:0x10a7 size:0x0006 C-string:'DABS'
//         DNEGATE  codep:0x10b9 wordp:0x10b9 size:0x000f C-string:'DNEGATE'
//              M*  codep:0x224c wordp:0x10cf size:0x0014 C-string:'M_star_'
//             M*/  codep:0x224c wordp:0x10eb size:0x0044 C-string:'M_star__slash_'
//              M+  codep:0x224c wordp:0x1136 size:0x0006 C-string:'M_plus_'
//              M/  codep:0x224c wordp:0x1143 size:0x0022 C-string:'M_slash_'
//           M/MOD  codep:0x224c wordp:0x116f size:0x0014 C-string:'M_slash_MOD'
//             MAX  codep:0x224c wordp:0x118b size:0x000e C-string:'MAX'
//             MOD  codep:0x224c wordp:0x11a1 size:0x0006 C-string:'MOD'
//             MIN  codep:0x224c wordp:0x11af size:0x000e C-string:'MIN'
//          NEGATE  codep:0x11c8 wordp:0x11c8 size:0x0009 C-string:'NEGATE'
//              U*  codep:0x11d8 wordp:0x11d8 size:0x000b C-string:'U_star_'
//           U/MOD  codep:0x11ed wordp:0x11ed size:0x000c C-string:'U_slash_MOD'
//            UMAX  codep:0x224c wordp:0x1202 size:0x000e C-string:'UMAX'
//            UMIN  codep:0x224c wordp:0x1219 size:0x0010 C-string:'UMIN'
//               >  codep:0x122f wordp:0x122f size:0x0013 C-string:'_gt_'
//               <  codep:0x1248 wordp:0x1248 size:0x0013 C-string:'_st_'
//               =  codep:0x1261 wordp:0x1261 size:0x0012 C-string:'_eq_'
//              0<  codep:0x127a wordp:0x127a size:0x000a C-string:'_0_st_'
//              0=  codep:0x128b wordp:0x128b size:0x000f C-string:'_0_eq_'
//              0>  codep:0x12a1 wordp:0x12a1 size:0x000c C-string:'_0_gt_'
//              D<  codep:0x224c wordp:0x12b4 size:0x0026 C-string:'D_st_'
//              U<  codep:0x12e1 wordp:0x12e1 size:0x000e C-string:'U_st_'
//             AND  codep:0x12f7 wordp:0x12f7 size:0x000a C-string:'AND'
//             NOT  codep:0x1309 wordp:0x1309 size:0x000f C-string:'NOT'
//           OFF_1  codep:0x224c wordp:0x1320 size:0x0008 C-string:'OFF_1'
//            ON_3  codep:0x224c wordp:0x132f size:0x000a C-string:'ON_3'
//              OR  codep:0x1340 wordp:0x1340 size:0x000a C-string:'OR'
//          TOGGLE  codep:0x1355 wordp:0x1355 size:0x0009 C-string:'TOGGLE'
//             XOR  codep:0x1366 wordp:0x1366 size:0x000a C-string:'XOR'
//               #  codep:0x224c wordp:0x1376 size:0x0024 C-string:'_n_'
//              #>  codep:0x224c wordp:0x13a1 size:0x000e C-string:'_n__gt_'
//              #S  codep:0x224c wordp:0x13b6 size:0x000e C-string:'_n_S'
//        (NUMBER)  codep:0x224c wordp:0x13d1 size:0x005a C-string:'_ro_NUMBER_rc_'
//      >UPPERCASE  codep:0x143a wordp:0x143a size:0x001e C-string:'_gt_UPPERCASE'
//              <#  codep:0x224c wordp:0x145f size:0x0008 C-string:'_st__n_'
//         CONVERT  codep:0x224c wordp:0x1473 size:0x0040 C-string:'CONVERT'
//           DIGIT  codep:0x14bd wordp:0x14bd size:0x0029 C-string:'DIGIT'
//            HOLD  codep:0x224c wordp:0x14ef size:0x0010 C-string:'HOLD'
//            S->D  codep:0x1508 wordp:0x1508 size:0x000b C-string:'S_dash__gt_D'
//            SIGN  codep:0x224c wordp:0x151c size:0x000e C-string:'SIGN'
//         (>CODE)  codep:0x1536 wordp:0x1536 size:0x0007 C-string:'_ro__gt_CODE_rc_'
//          (;VIA)  codep:0x1548 wordp:0x1548 size:0x000a C-string:'_ro__sc_VIA_rc_'
//         (+LOOP)  codep:0x155e wordp:0x155e size:0x0029 C-string:'_ro__plus_LOOP_rc_'
//         (/LOOP)  codep:0x1593 wordp:0x1593 size:0x001e C-string:'_ro__slash_LOOP_rc_'
//            (DO)  codep:0x15ba wordp:0x15ba size:0x000d C-string:'_ro_DO_rc_'
//          (LOOP)  codep:0x15d2 wordp:0x15d2 size:0x001e C-string:'_ro_LOOP_rc_'
//         0BRANCH  codep:0x15fc wordp:0x15fc size:0x0013 C-string:'_0BRANCH'
//            2LIT  codep:0x1618 wordp:0x1618 size:0x0009 C-string:'_2LIT'
//        ABORT-IT  codep:0x224c wordp:0x162e size:0x0029 C-string:'ABORT_dash_IT'
//          BRANCH  codep:0x1662 wordp:0x1662 size:0x0007 C-string:'BRANCH'
//         CFAEXEC  codep:0x1675 wordp:0x1675 size:0x0003 C-string:'CFAEXEC'
//         EXECUTE  codep:0x1684 wordp:0x1684 size:0x0005 C-string:'EXECUTE'
//            EXIT  codep:0x1692 wordp:0x1692 size:0x000a C-string:'EXIT'
//              GO  codep:0x16a3 wordp:0x16a3 size:0x0003 C-string:'GO'
//       REGISTERS  codep:0x1d29 wordp:0x16b4 size:0x0017 C-string:'REGISTERS'
//       INTERRUPT  codep:0x16d9 wordp:0x16d9 size:0x0069 C-string:'INTERRUPT'
//           LEAVE  codep:0x174c wordp:0x174c size:0x000b C-string:'LEAVE'
//             LIT  codep:0x175f wordp:0x175f size:0x0007 C-string:'LIT'
//         SYSCALL  codep:0x1772 wordp:0x1772 size:0x000f C-string:'SYSCALL'
//            USER  codep:0x224c wordp:0x178a size:0x0014 C-string:'USER'
//    USEREXECUTOR  codep:0x224c wordp:0x17af size:0x0012 C-string:'USEREXECUTOR'
//           +FIND  codep:0x224c wordp:0x17cb size:0x000a C-string:'_plus_FIND'
//         (-FIND)  codep:0x224c wordp:0x17e1 size:0x002c C-string:'_ro__dash_FIND_rc_'
//          (FIND)  codep:0x1818 wordp:0x1818 size:0x0049 C-string:'_ro_FIND_rc_'
//        (FORGET)  codep:0x224c wordp:0x186e size:0x00a0 C-string:'_ro_FORGET_rc_'
//         (LINKS)  codep:0x224c wordp:0x191a size:0x001a C-string:'_ro_LINKS_rc_'
//          <NAME>  codep:0x224c wordp:0x193f size:0x0018 C-string:'_st_NAME_gt_'
//           ?FIND  codep:0x224c wordp:0x1961 size:0x0016 C-string:'IsFIND'
//          ?[PTR]  codep:0x224c wordp:0x1982 size:0x000a C-string:'Is_bo_PTR_bc_'
//     DEFINITIONS  codep:0x224c wordp:0x199c size:0x000a C-string:'DEFINITIONS'
//           EMPTY  codep:0x224c wordp:0x19b0 size:0x001a C-string:'EMPTY'
//            FIND  codep:0x224c wordp:0x19d3 size:0x0014 C-string:'FIND'
//          FORGET  codep:0x224c wordp:0x19f2 size:0x0068 C-string:'FORGET'
//           W1A5C  codep:0x224c wordp:0x1a5c size:0x0008 C-string:'W1A5C'
//           W1A66  codep:0x224c wordp:0x1a66 size:0x0006 C-string:'W1A66'
//           W1A6E  codep:0x224c wordp:0x1a6e size:0x000c C-string:'W1A6E'
//      VOCABULARY  codep:0x224c wordp:0x1a89 size:0x0035 C-string:'VOCABULARY'
//           W1AC0  codep:0x1ac0 wordp:0x1ac0 size:0x0014 C-string:'W1AC0'
//         WITHIN?  codep:0x224c wordp:0x1ae0 size:0x0020 C-string:'WITHIN_ask_'
//         [-FIND]  codep:0x224c wordp:0x1b0c size:0x001c C-string:'_bo__dash_FIND_bc_'
//        [-FINDS]  codep:0x224c wordp:0x1b35 size:0x0042 C-string:'_bo__dash_FINDS_bc_'
//             '_1  codep:0x224c wordp:0x1b7d size:0x0006 C-string:'_i__1'
//         'STREAM  codep:0x224c wordp:0x1b8f size:0x001c C-string:'_i_STREAM'
//               (  codep:0x224c wordp:0x1bb1 size:0x0024 C-string:'_ro_'
//            (.")  codep:0x224c wordp:0x1bde size:0x0012 C-string:'_ro__dot__qm__rc_'
//         (;CODE)  codep:0x224c wordp:0x1bfc size:0x000c C-string:'_ro__sc_CODE_rc_'
//        (ABORT")  codep:0x224c wordp:0x1c15 size:0x0024 C-string:'_ro_ABORT_qm__rc_'
//         (ABORT)  codep:0x224c wordp:0x1c45 size:0x000a C-string:'_ro_ABORT_rc_'
//              ."  codep:0x224c wordp:0x1c56 size:0x0058 C-string:'Draw_qm_'
//        (CREATE)  codep:0x224c wordp:0x1cbb size:0x0076 C-string:'_ro_CREATE_rc_'
//        (EXPECT)  codep:0x224c wordp:0x1d3e size:0x008a C-string:'_ro_EXPECT_rc_'
//     (INTERPRET)  codep:0x224c wordp:0x1dd8 size:0x0040 C-string:'_ro_INTERPRET_rc_'
//          (LOAD)  codep:0x224c wordp:0x1e23 size:0x0039 C-string:'_ro_LOAD_rc_'
//         (WHERE)  codep:0x224c wordp:0x1e68 size:0x0093 C-string:'_ro_WHERE_rc_'
//          (WORD)  codep:0x224c wordp:0x1f06 size:0x006d C-string:'_ro_WORD_rc_'
//           +LOOP  codep:0x224c wordp:0x1f7d size:0x0012 C-string:'_plus_LOOP'
//               ,  codep:0x224c wordp:0x1f95 size:0x000e C-string:'_co_'
//           /LOOP  codep:0x224c wordp:0x1fad size:0x0012 C-string:'_slash_LOOP'
//               ;  codep:0x224c wordp:0x1fc5 size:0x000c C-string:'_sc_'
//           ?COMP  codep:0x224c wordp:0x1fdb size:0x0017 C-string:'IsCOMP'
//           W1FF4  codep:0x224c wordp:0x1ff4 size:0x0034 C-string:'W1FF4'
//            ?CSP  codep:0x224c wordp:0x2031 size:0x0024 C-string:'IsCSP'
//           W2057  codep:0x224c wordp:0x2057 size:0x0020 C-string:'W2057'
//        ?LOADING  codep:0x224c wordp:0x2084 size:0x0017 C-string:'IsLOADING'
//          ?PAIRS  codep:0x224c wordp:0x20a6 size:0x001e C-string:'IsPAIRS'
//           W20C6  codep:0x224c wordp:0x20c6 size:0x0055 C-string:'W20C6'
//          ABORT"  codep:0x224c wordp:0x2126 size:0x0044 C-string:'ABORT_qm_'
//           AGAIN  codep:0x224c wordp:0x2174 size:0x0010 C-string:'AGAIN'
//           ALLOT  codep:0x224c wordp:0x218e size:0x000a C-string:'ALLOT'
//           BEGIN  codep:0x224c wordp:0x21a2 size:0x0008 C-string:'BEGIN'
//              C,  codep:0x224c wordp:0x21b1 size:0x000a C-string:'C_co_'
//             CFA  codep:0x224c wordp:0x21c3 size:0x0004 C-string:'CFA'
//           W21C9  codep:0x21c9 wordp:0x21c9 size:0x001c C-string:'W21C9'
//         COMPILE  codep:0x224c wordp:0x21f1 size:0x0010 C-string:'COMPILE'
//        CONSTANT  codep:0x224c wordp:0x220e size:0x000f C-string:'CONSTANT'
//              DO  codep:0x224c wordp:0x2224 size:0x000c C-string:'DO'
//               :  codep:0x224c wordp:0x2236 size:0x0024 C-string:'_c_'
//           DOES>  codep:0x224c wordp:0x2264 size:0x001a C-string:'DOES_gt_'
//            ELSE  codep:0x224c wordp:0x2287 size:0x0018 C-string:'ELSE'
//         ENCLOSE  codep:0x22ab wordp:0x22ab size:0x003b C-string:'ENCLOSE'
//            HERE  codep:0x224c wordp:0x22ef size:0x0006 C-string:'HERE'
//             HEX  codep:0x224c wordp:0x22fd size:0x000a C-string:'HEX'
//              IF  codep:0x224c wordp:0x230e size:0x000e C-string:'IF'
//       IMMEDIATE  codep:0x224c wordp:0x232a size:0x000a C-string:'IMMEDIATE'
//          LATEST  codep:0x224c wordp:0x233f size:0x000e C-string:'LATEST'
//             LFA  codep:0x224c wordp:0x2355 size:0x0006 C-string:'LFA'
//         LITERAL  codep:0x224c wordp:0x2367 size:0x0016 C-string:'LITERAL'
//            LOOP  codep:0x224c wordp:0x2386 size:0x0012 C-string:'LOOP'
//        2LITERAL  codep:0x224c wordp:0x23a5 size:0x001a C-string:'_2LITERAL'
//             NFA  codep:0x224c wordp:0x23c7 size:0x000e C-string:'NFA'
//             PAD  codep:0x224c wordp:0x23dd size:0x000a C-string:'PAD'
//             PFA  codep:0x224c wordp:0x23ef size:0x000c C-string:'PFA'
//           QUERY  codep:0x224c wordp:0x2405 size:0x0012 C-string:'QUERY'
//            QUIT  codep:0x224c wordp:0x2420 size:0x003c C-string:'QUIT'
//          REPEAT  codep:0x224c wordp:0x2467 size:0x0010 C-string:'REPEAT'
//          SMUDGE  codep:0x224c wordp:0x2482 size:0x000a C-string:'SMUDGE'
//            TEXT  codep:0x224c wordp:0x2495 size:0x0022 C-string:'TEXT'
//            THEN  codep:0x224c wordp:0x24c0 size:0x0012 C-string:'THEN'
//        TRAVERSE  codep:0x224c wordp:0x24df size:0x001a C-string:'TRAVERSE'
//           UNTIL  codep:0x224c wordp:0x2503 size:0x0010 C-string:'UNTIL'
//        VARIABLE  codep:0x224c wordp:0x2520 size:0x0008 C-string:'VARIABLE'
//           WHILE  codep:0x224c wordp:0x2532 size:0x0006 C-string:'WHILE'
// zero_length_unknown  codep:0x224c wordp:0x253e size:0x0014 C-string:'zero_length_unknown'
//               [  codep:0x224c wordp:0x2558 size:0x0008 C-string:'_bo_'
//       [COMPILE]  codep:0x224c wordp:0x256e size:0x000a C-string:'_bo_COMPILE_bc_'
//               \  codep:0x224c wordp:0x257e size:0x001e C-string:'_bs_'
//               ]  codep:0x224c wordp:0x25a2 size:0x000a C-string:'_bc_'
//     (?TERMINAL)  codep:0x25bc wordp:0x25bc size:0x0011 C-string:'_ro__ask_TERMINAL_rc_'
//           (KEY)  codep:0x25d7 wordp:0x25d7 size:0x001c C-string:'_ro_KEY_rc_'
//          BEEPMS  codep:0x1d29 wordp:0x25fe size:0x0002 C-string:'BEEPMS'
//        BEEPTONE  codep:0x1d29 wordp:0x260d size:0x0002 C-string:'BEEPTONE'
//            TONE  codep:0x2618 wordp:0x2618 size:0x0015 C-string:'TONE'
//        BEEPON_1  codep:0x2638 wordp:0x2638 size:0x000f C-string:'BEEPON_1'
//         BEEPOFF  codep:0x2653 wordp:0x2653 size:0x000f C-string:'BEEPOFF'
//            BEEP  codep:0x224c wordp:0x266b size:0x001a C-string:'BEEP'
//          (TYPE)  codep:0x224c wordp:0x2690 size:0x0036 C-string:'_ro_TYPE_rc_'
//            (BS)  codep:0x224c wordp:0x26cf size:0x0016 C-string:'_ro_BS_rc_'
//            (CR)  codep:0x224c wordp:0x26ee size:0x0038 C-string:'_ro_CR_rc_'
//          (EMIT)  codep:0x224c wordp:0x2731 size:0x0010 C-string:'_ro_EMIT_rc_'
//          (PAGE)  codep:0x224c wordp:0x274c size:0x000c C-string:'_ro_PAGE_rc_'
//      (POSITION)  codep:0x2767 wordp:0x2767 size:0x001b C-string:'_ro_POSITION_rc_'
//       -TRAILING  codep:0x224c wordp:0x2790 size:0x0024 C-string:'_dash_TRAILING'
//               .  codep:0x224c wordp:0x27ba size:0x0006 C-string:'Draw'
//           .LINE  codep:0x224c wordp:0x27ca size:0x0012 C-string:'DrawLINE'
//              .R  codep:0x224c wordp:0x27e3 size:0x000a C-string:'DrawR'
//          >VMOVE  codep:0x27f8 wordp:0x27f8 size:0x0024 C-string:'_gt_VMOVE'
//               ?  codep:0x224c wordp:0x2822 size:0x0006 C-string:'Is'
//       ?POSITION  codep:0x2836 wordp:0x2836 size:0x000f C-string:'IsPOSITION'
//        CUR>ADDR  codep:0x2852 wordp:0x2852 size:0x000b C-string:'CUR_gt_ADDR'
//          CURFWD  codep:0x224c wordp:0x2868 size:0x0026 C-string:'CURFWD'
//              D.  codep:0x224c wordp:0x2895 size:0x0008 C-string:'D_dot_'
//             D.R  codep:0x224c wordp:0x28a5 size:0x0026 C-string:'D_dot_R'
//             ID.  codep:0x224c wordp:0x28d3 size:0x004c C-string:'ID_dot_'
//            LIST  codep:0x224c wordp:0x2928 size:0x003e C-string:'LIST'
//        SCROLLUP  codep:0x2973 wordp:0x2973 size:0x0019 C-string:'SCROLLUP'
//           SPACE  codep:0x224c wordp:0x2996 size:0x0006 C-string:'SPACE'
//          SPACES  codep:0x224c wordp:0x29a7 size:0x0016 C-string:'SPACES'
//              U.  codep:0x224c wordp:0x29c4 size:0x0006 C-string:'U_dot_'
//             U.R  codep:0x224c wordp:0x29d2 size:0x0008 C-string:'U_dot_R'
//              V@  codep:0x29e1 wordp:0x29e1 size:0x0014 C-string:'V_at_'
//              V!  codep:0x29fc wordp:0x29fc size:0x0012 C-string:'V_ex_'
//           VFILL  codep:0x2a18 wordp:0x2a18 size:0x0014 C-string:'VFILL'
//           VMOVE  codep:0x2a36 wordp:0x2a36 size:0x001e C-string:'VMOVE'
//          VMOVE-  codep:0x2a5f wordp:0x2a5f size:0x0007 C-string:'VMOVE_dash_'
//         MS/TICK  codep:0x2214 wordp:0x2a72 size:0x0002 C-string:'MS_slash_TICK'
//       TICKS/ADJ  codep:0x2214 wordp:0x2a82 size:0x0002 C-string:'TICKS_slash_ADJ'
//          MS/ADJ  codep:0x2214 wordp:0x2a8f size:0x0002 C-string:'MS_slash_ADJ'
//            TIME  codep:0x2a9a wordp:0x2a9a size:0x0050 C-string:'TIME'
//              MS  codep:0x224c wordp:0x2af1 size:0x0024 C-string:'MS'
//         #DRIVES  codep:0x2214 wordp:0x2b21 size:0x0002 C-string:'_n_DRIVES'
//           #FCBS  codep:0x2214 wordp:0x2b2d size:0x0002 C-string:'_n_FCBS'
//          #FILES  codep:0x2214 wordp:0x2b3a size:0x0002 C-string:'_n_FILES'
//        #SEC/TRK  codep:0x2214 wordp:0x2b49 size:0x0002 C-string:'_n_SEC_slash_TRK'
//        DOS-FILE  codep:0x2214 wordp:0x2b58 size:0x0002 C-string:'DOS_dash_FILE'
//         RETRIES  codep:0x2214 wordp:0x2b66 size:0x0002 C-string:'RETRIES'
//       SECORIGIN  codep:0x2214 wordp:0x2b76 size:0x0002 C-string:'SECORIGIN'
//          'SVBUF  codep:0x1d29 wordp:0x2b83 size:0x0002 C-string:'_i_SVBUF'
//          'MTBUF  codep:0x1d29 wordp:0x2b90 size:0x0002 C-string:'_i_MTBUF'
//           #SECS  codep:0x1d29 wordp:0x2b9c size:0x0002 C-string:'_n_SECS'
//            'FCB  codep:0x1d29 wordp:0x2ba7 size:0x0002 C-string:'_i_FCB'
//          'CACHE  codep:0x1d29 wordp:0x2bb4 size:0x0002 C-string:'_i_CACHE'
//       'DOSMOUNT  codep:0x1d29 wordp:0x2bc4 size:0x0002 C-string:'_i_DOSMOUNT'
//     'DOSUNMOUNT  codep:0x1d29 wordp:0x2bd6 size:0x0002 C-string:'_i_DOSUNMOUNT'
//          REUSEC  codep:0x1d29 wordp:0x2be3 size:0x0002 C-string:'REUSEC'
//     1STCACHEBUF  codep:0x1d29 wordp:0x2bf5 size:0x0002 C-string:'_1STCACHEBUF'
//        BLKCACHE  codep:0x1d29 wordp:0x2c04 size:0x0002 C-string:'BLKCACHE'
//      DEFAULTDRV  codep:0x1d29 wordp:0x2c15 size:0x0002 C-string:'DEFAULTDRV'
//      DISK-ERROR  codep:0x1d29 wordp:0x2c26 size:0x0002 C-string:'DISK_dash_ERROR'
//           DRIVE  codep:0x1d29 wordp:0x2c32 size:0x0002 C-string:'DRIVE'
//           DTA_1  codep:0x1d29 wordp:0x2c3c size:0x0004 C-string:'DTA_1'
//            FILE  codep:0x1d29 wordp:0x2c49 size:0x0002 C-string:'FILE'
//           FRESH  codep:0x1d29 wordp:0x2c55 size:0x0002 C-string:'FRESH'
//            HEAD  codep:0x1d29 wordp:0x2c60 size:0x0002 C-string:'HEAD'
//           LPREV  codep:0x1d29 wordp:0x2c6c size:0x0002 C-string:'LPREV'
//          OFFSET  codep:0x1d29 wordp:0x2c79 size:0x0002 C-string:'OFFSET'
//            PREV  codep:0x1d29 wordp:0x2c84 size:0x0002 C-string:'PREV'
//             SEC  codep:0x1d29 wordp:0x2c8e size:0x0002 C-string:'SEC'
//        SEGCACHE  codep:0x1d29 wordp:0x2c9d size:0x0002 C-string:'SEGCACHE'
//            SYSK  codep:0x1d29 wordp:0x2ca8 size:0x0002 C-string:'SYSK'
//           TRACK  codep:0x1d29 wordp:0x2cb4 size:0x0002 C-string:'TRACK'
//             USE  codep:0x1d29 wordp:0x2cbe size:0x0002 C-string:'USE'
//     =DRIVESIZES  codep:0x1d29 wordp:0x2cd0 size:0x0006 C-string:'_eq_DRIVESIZES'
//      DRIVESIZES  codep:0x224c wordp:0x2ce5 size:0x000a C-string:'DRIVESIZES'
//          BARRAY  codep:0x224c wordp:0x2cfa size:0x0004 C-string:'BARRAY'
//          WARRAY  codep:0x224c wordp:0x2d09 size:0x0008 C-string:'WARRAY'
//   =DRIVENUMBERS  codep:0x1d29 wordp:0x2d23 size:0x0004 C-string:'_eq_DRIVENUMBERS'
//        =FCBPFAS  codep:0x1d29 wordp:0x2d34 size:0x0008 C-string:'_eq_FCBPFAS'
//     =FILEBLOCKS  codep:0x1d29 wordp:0x2d4c size:0x0008 C-string:'_eq_FILEBLOCKS'
//      =FILETYPES  codep:0x1d29 wordp:0x2d63 size:0x0004 C-string:'_eq_FILETYPES'
//        =OFFSETS  codep:0x1d29 wordp:0x2d74 size:0x0008 C-string:'_eq_OFFSETS'
//          =R/W'S  codep:0x1d29 wordp:0x2d87 size:0x0008 C-string:'_eq_R_slash_W_i_S'
//             FCB  codep:0x1d29 wordp:0x2d97 size:0x0058 C-string:'FCB'
//       FILENAMES  codep:0x1d29 wordp:0x2dfd size:0x002c C-string:'FILENAMES'
//    DRIVENUMBERS  codep:0x224c wordp:0x2e3a size:0x0006 C-string:'DRIVENUMBERS'
//         FCBPFAS  codep:0x224c wordp:0x2e4c size:0x0006 C-string:'FCBPFAS'
//      FILEBLOCKS  codep:0x224c wordp:0x2e61 size:0x0006 C-string:'FILEBLOCKS'
//       FILETYPES  codep:0x224c wordp:0x2e75 size:0x0006 C-string:'FILETYPES'
//         OFFSETS  codep:0x224c wordp:0x2e87 size:0x0006 C-string:'OFFSETS'
//           R/W'S  codep:0x224c wordp:0x2e97 size:0x0006 C-string:'R_slash_W_i_S'
//              L@  codep:0x2ea4 wordp:0x2ea4 size:0x000d C-string:'L_at_'
//              L!  codep:0x2eb8 wordp:0x2eb8 size:0x000d C-string:'L_ex_'
//             LC@  codep:0x2ecd wordp:0x2ecd size:0x0010 C-string:'LC_at_'
//             LC!  codep:0x2ee5 wordp:0x2ee5 size:0x000e C-string:'LC_ex_'
//          LCMOVE  codep:0x2efe wordp:0x2efe size:0x002c C-string:'LCMOVE'
//         {LXCHG}  codep:0x1d29 wordp:0x2f36 size:0x0010 C-string:'_co_LXCHG_cc_'
//          LWSCAN  codep:0x2f51 wordp:0x2f51 size:0x0029 C-string:'LWSCAN'
//      [SEGCACHE]  codep:0x224c wordp:0x2f89 size:0x000e C-string:'_bo_SEGCACHE_bc_'
//      [BLKCACHE]  codep:0x224c wordp:0x2fa6 size:0x000e C-string:'_bo_BLKCACHE_bc_'
//          UPDATE  codep:0x224c wordp:0x2fbf size:0x000e C-string:'UPDATE'
//         REFRESH  codep:0x224c wordp:0x2fd9 size:0x0006 C-string:'REFRESH'
//           MTBUF  codep:0x224c wordp:0x2fe9 size:0x0036 C-string:'MTBUF'
//       MTBUFFERS  codep:0x224c wordp:0x302d size:0x000e C-string:'MTBUFFERS'
//        (BUFFER)  codep:0x3048 wordp:0x3048 size:0x0054 C-string:'_ro_BUFFER_rc_'
//         ADVANCE  codep:0x30a8 wordp:0x30a8 size:0x0021 C-string:'ADVANCE'
//         MTCACHE  codep:0x224c wordp:0x30d5 size:0x0032 C-string:'MTCACHE'
//       INITCACHE  codep:0x224c wordp:0x3115 size:0x0056 C-string:'INITCACHE'
//         (CACHE)  codep:0x224c wordp:0x3177 size:0x0036 C-string:'_ro_CACHE_rc_'
//       CACHE>USE  codep:0x224c wordp:0x31bb size:0x001c C-string:'CACHE_gt_USE'
//       ?INBCACHE  codep:0x224c wordp:0x31e5 size:0x0026 C-string:'IsINBCACHE'
//     [BUF>CACHE]  codep:0x224c wordp:0x321b size:0x001c C-string:'_bo_BUF_gt_CACHE_bc_'
//       BUF>CACHE  codep:0x224c wordp:0x3245 size:0x001a C-string:'BUF_gt_CACHE'
//         SEC-I/O  codep:0x224c wordp:0x326b size:0x005e C-string:'SEC_dash_I_slash_O'
//         T&SCALC  codep:0x224c wordp:0x32d5 size:0x001e C-string:'T_and_SCALC'
//           [R/W]  codep:0x224c wordp:0x32fd size:0x003a C-string:'_bo_R_slash_W_bc_'
//        [DIRECT]  codep:0x224c wordp:0x3344 size:0x0048 C-string:'_bo_DIRECT_bc_'
//         <MOUNT>  codep:0x224c wordp:0x3398 size:0x0064 C-string:'_st_MOUNT_gt_'
//         DIRNAME  codep:0x1d29 wordp:0x3408 size:0x000b C-string:'DIRNAME'
//             DR0  codep:0x224c wordp:0x341b size:0x0016 C-string:'DR0'
//             DR1  codep:0x224c wordp:0x3439 size:0x0018 C-string:'DR1'
//       <UNMOUNT>  codep:0x224c wordp:0x345f size:0x005e C-string:'_st_UNMOUNT_gt_'
//         UNMOUNT  codep:0x224c wordp:0x34c9 size:0x0017 C-string:'UNMOUNT'
//         NOFILES  codep:0x224c wordp:0x34ec size:0x0014 C-string:'NOFILES'
//        NODRIVES  codep:0x224c wordp:0x350d size:0x0004 C-string:'NODRIVES'
//           EMITS  codep:0x224c wordp:0x351b size:0x0016 C-string:'EMITS'
//           FILES  codep:0x224c wordp:0x353b size:0x00ed C-string:'FILES'
//          DRIVES  codep:0x224c wordp:0x3633 size:0x0004 C-string:'DRIVES'
//    (DISKERROR?)  codep:0x224c wordp:0x3648 size:0x0028 C-string:'_ro_DISKERROR_ask__rc_'
//           W3672  codep:0x3672 wordp:0x3672 size:0x000b C-string:'W3672'
//           W367F  codep:0x367f wordp:0x367f size:0x003a C-string:'W367F'
//           W36BB  codep:0x36bb wordp:0x36bb size:0x001a C-string:'W36BB'
//           W36D7  codep:0x224c wordp:0x36d7 size:0x003c C-string:'W36D7'
//           W3715  codep:0x224c wordp:0x3715 size:0x003c C-string:'W3715'
//           W3753  codep:0x224c wordp:0x3753 size:0x002e C-string:'W3753'
//           W3783  codep:0x224c wordp:0x3783 size:0x002a C-string:'W3783'
//           W37AF  codep:0x224c wordp:0x37af size:0x0022 C-string:'W37AF'
//           W37D3  codep:0x224c wordp:0x37d3 size:0x004e C-string:'W37D3'
//           W3823  codep:0x224c wordp:0x3823 size:0x0024 C-string:'W3823'
//           W3849  codep:0x224c wordp:0x3849 size:0x000c C-string:'W3849'
//         BLOCK_1  codep:0x224c wordp:0x385f size:0x0016 C-string:'BLOCK_1'
//        LBLOCK_1  codep:0x224c wordp:0x3880 size:0x001c C-string:'LBLOCK_1'
//    SAVE-BUFFERS  codep:0x224c wordp:0x38ad size:0x003e C-string:'SAVE_dash_BUFFERS'
//   EMPTY-BUFFERS  codep:0x224c wordp:0x38fd size:0x000c C-string:'EMPTY_dash_BUFFERS'
//         FLUSH_1  codep:0x224c wordp:0x3913 size:0x0006 C-string:'FLUSH_1'
//            COPY  codep:0x224c wordp:0x3922 size:0x0012 C-string:'COPY'
//          BLOCKS  codep:0x224c wordp:0x393f size:0x004a C-string:'BLOCKS'
//             PSW  codep:0x2214 wordp:0x3991 size:0x0002 C-string:'PSW'
//              AX  codep:0x2214 wordp:0x399a size:0x0002 C-string:'AX'
//              BX  codep:0x2214 wordp:0x39a3 size:0x0002 C-string:'BX'
//              CX  codep:0x2214 wordp:0x39ac size:0x0002 C-string:'CX'
//              DX  codep:0x2214 wordp:0x39b5 size:0x0002 C-string:'DX'
//              BP  codep:0x2214 wordp:0x39be size:0x0002 C-string:'BP'
//              DI  codep:0x2214 wordp:0x39c7 size:0x0002 C-string:'DI'
//              SI  codep:0x2214 wordp:0x39d0 size:0x0002 C-string:'SI'
//              DS  codep:0x2214 wordp:0x39d9 size:0x0002 C-string:'DS'
//              ES  codep:0x2214 wordp:0x39e2 size:0x0002 C-string:'ES'
//           MATCH  codep:0x39ee wordp:0x39ee size:0x003e C-string:'MATCH'
//        @EXECUTE  codep:0x3a39 wordp:0x3a39 size:0x0007 C-string:'GetEXECUTE'
//             NOP  codep:0x224c wordp:0x3a48 size:0x0002 C-string:'NOP'
//              **  codep:0x224c wordp:0x3a51 size:0x0002 C-string:'_star__star_'
//        'UNRAVEL  codep:0x1d29 wordp:0x3a60 size:0x0002 C-string:'_i_UNRAVEL'
//         UNRAVEL  codep:0x224c wordp:0x3a6e size:0x0044 C-string:'UNRAVEL'
//        ?UNRAVEL  codep:0x224c wordp:0x3abf size:0x0008 C-string:'IsUNRAVEL'
//             SYN  codep:0x224c wordp:0x3acf size:0x0042 C-string:'SYN'
//           TRACE  codep:0x1d29 wordp:0x3b1b size:0x0002 C-string:'TRACE'
//          [TRACE  codep:0x1d29 wordp:0x3b28 size:0x0002 C-string:'_bo_TRACE'
//          TRACE]  codep:0x1d29 wordp:0x3b35 size:0x0002 C-string:'TRACE_bc_'
//              D@  codep:0x3af8 wordp:0x3b3e size:0x0002 C-string:'GetD'
//              V=  codep:0x3af8 wordp:0x3b47 size:0x0002 C-string:'V_eq_'
//              C=  codep:0x3af8 wordp:0x3b50 size:0x0002 C-string:'C_eq_'
//             2V=  codep:0x224c wordp:0x3b5a size:0x000a C-string:'_2V_eq_'
//             2C=  codep:0x224c wordp:0x3b6c size:0x000f C-string:'_2C_eq_'
//               3  codep:0x2214 wordp:0x3b81 size:0x0002 C-string:'_3'
//               4  codep:0x2214 wordp:0x3b89 size:0x0002 C-string:'_4'
//               5  codep:0x2214 wordp:0x3b91 size:0x0002 C-string:'_5'
//               6  codep:0x2214 wordp:0x3b99 size:0x0002 C-string:'_6'
//               7  codep:0x2214 wordp:0x3ba1 size:0x0002 C-string:'_7'
//               8  codep:0x2214 wordp:0x3ba9 size:0x0002 C-string:'_8'
//               9  codep:0x2214 wordp:0x3bb1 size:0x0002 C-string:'_9'
//              -1  codep:0x2214 wordp:0x3bba size:0x0002 C-string:'_dash_1'
//              -2  codep:0x2214 wordp:0x3bc3 size:0x0002 C-string:'_dash_2'
//          THRU_1  codep:0x224c wordp:0x3bce size:0x0020 C-string:'THRU_1'
//         (TRACE)  codep:0x224c wordp:0x3bfa size:0x0002 C-string:'_ro_TRACE_rc_'
//             OVA  codep:0x1d29 wordp:0x3c04 size:0x0002 C-string:'OVA'
//            T-DP  codep:0x1d29 wordp:0x3c0f size:0x0006 C-string:'T_dash_DP'
//     TRANS-ALLOT  codep:0x224c wordp:0x3c25 size:0x000e C-string:'TRANS_dash_ALLOT'
//         NEWT-DP  codep:0x224c wordp:0x3c3f size:0x000e C-string:'NEWT_dash_DP'
//       TRANSIENT  codep:0x224c wordp:0x3c5b size:0x0016 C-string:'TRANSIENT'
//        RESIDENT  codep:0x224c wordp:0x3c7e size:0x0016 C-string:'RESIDENT'
//           HEAD:  codep:0x224c wordp:0x3c9e size:0x0027 C-string:'HEAD_c_'
//              T:  codep:0x224c wordp:0x3ccc size:0x0018 C-string:'T_c_'
//              T;  codep:0x224c wordp:0x3ceb size:0x000a C-string:'T_sc_'
//           W3CF7  codep:0x224c wordp:0x3cf7 size:0x004c C-string:'W3CF7'
//           W3D45  codep:0x224c wordp:0x3d45 size:0x004a C-string:'W3D45'
//         DISPOSE  codep:0x224c wordp:0x3d9b size:0x0062 C-string:'DISPOSE'
//             '_2  codep:0x224c wordp:0x3e03 size:0x0020 C-string:'_i__2'
//           (2V:)  codep:0x1d29 wordp:0x3e2d size:0x0004 C-string:'_ro_2V_c__rc_'
//            (C:)  codep:0x2214 wordp:0x3e3a size:0x0002 C-string:'_ro_C_c__rc_'
//           (2C:)  codep:0x3b74 wordp:0x3e46 size:0x0004 C-string:'_ro_2C_c__rc_'
//              V:  codep:0x224c wordp:0x3e51 size:0x000e C-string:'V_c_'
//             2V:  codep:0x224c wordp:0x3e67 size:0x000e C-string:'_2V_c_'
//              C:  codep:0x224c wordp:0x3e7c size:0x000c C-string:'C_c_'
//             2C:  codep:0x224c wordp:0x3e90 size:0x000e C-string:'_2C_c_'
//        EXECUTES  codep:0x224c wordp:0x3eab size:0x000a C-string:'EXECUTES'
//          BINARY  codep:0x224c wordp:0x3ec0 size:0x0008 C-string:'BINARY'
//              H.  codep:0x224c wordp:0x3ecf size:0x0010 C-string:'H_dot_'
//           RANGE  codep:0x224c wordp:0x3ee9 size:0x0008 C-string:'RANGE'
//           ASCII  codep:0x224c wordp:0x3efb size:0x000c C-string:'ASCII'
// SET_STR_AS_PARAM  codep:0x224c wordp:0x3f09 size:0x0010 C-string:'SET_STR_AS_PARAM'
//               "  codep:0x224c wordp:0x3f1f size:0x0036 C-string:'_qm_'
//             DU<  codep:0x224c wordp:0x3f5d size:0x0018 C-string:'DU_st_'
//            PICK  codep:0x224c wordp:0x3f7e size:0x0016 C-string:'PICK'
//            ROLL  codep:0x224c wordp:0x3f9d size:0x0036 C-string:'ROLL'
//           2OVER  codep:0x224c wordp:0x3fdd size:0x000a C-string:'_2OVER'
//              D-  codep:0x224c wordp:0x3fee size:0x0006 C-string:'D_dash_'
//             D0=  codep:0x224c wordp:0x3ffc size:0x0006 C-string:'D0_eq_'
//              D=  codep:0x224c wordp:0x4009 size:0x0006 C-string:'D_eq_'
//              D>  codep:0x224c wordp:0x4016 size:0x0006 C-string:'D_gt_'
//          WITHIN  codep:0x224c wordp:0x4027 size:0x0014 C-string:'WITHIN'
//            DMIN  codep:0x224c wordp:0x4044 size:0x0010 C-string:'DMIN'
//            DMAX  codep:0x224c wordp:0x405d size:0x0010 C-string:'DMAX'
//          MYSELF  codep:0x224c wordp:0x4078 size:0x001c C-string:'MYSELF'
//            >TIB  codep:0x224c wordp:0x409d size:0x0014 C-string:'_gt_TIB'
//         DOSPARM  codep:0x224c wordp:0x40bd size:0x000c C-string:'DOSPARM'
//          MAXDRV  codep:0x1d29 wordp:0x40d4 size:0x0002 C-string:'MAXDRV'
//         ?#DETTE  codep:0x224c wordp:0x40e2 size:0x001a C-string:'Is_n_DETTE'
//            XFCB  codep:0x224c wordp:0x4105 size:0x000a C-string:'XFCB'
//             DRV  codep:0x224c wordp:0x4117 size:0x0006 C-string:'DRV'
//             NAM  codep:0x224c wordp:0x4125 size:0x0008 C-string:'NAM'
//             TYP  codep:0x224c wordp:0x4135 size:0x000c C-string:'TYP'
//          CURBLK  codep:0x224c wordp:0x414c size:0x000c C-string:'CURBLK'
//         RECSIZE  codep:0x224c wordp:0x4164 size:0x000c C-string:'RECSIZE'
//         FILESIZ  codep:0x224c wordp:0x417c size:0x000c C-string:'FILESIZ'
//          CURREC  codep:0x224c wordp:0x4193 size:0x000c C-string:'CURREC'
//          RELREC  codep:0x224c wordp:0x41aa size:0x000c C-string:'RELREC'
//          CLRFCB  codep:0x224c wordp:0x41c1 size:0x001c C-string:'CLRFCB'
//          SYSTEM  codep:0x224c wordp:0x41e8 size:0x000e C-string:'SYSTEM'
//         SYSUTIL  codep:0x224c wordp:0x4202 size:0x0010 C-string:'SYSUTIL'
//         DOSCALL  codep:0x224c wordp:0x421e size:0x000e C-string:'DOSCALL'
//            >FCB  codep:0x224c wordp:0x4235 size:0x0034 C-string:'_gt_FCB'
//          SETFCB  codep:0x224c wordp:0x4274 size:0x0024 C-string:'SETFCB'
//            INIT  codep:0x224c wordp:0x42a1 size:0x0008 C-string:'INIT'
//         FCBCALL  codep:0x224c wordp:0x42b5 size:0x0016 C-string:'FCBCALL'
//            OPEN  codep:0x224c wordp:0x42d4 size:0x0008 C-string:'OPEN'
//           CLOSE  codep:0x224c wordp:0x42e6 size:0x0008 C-string:'CLOSE'
//         SEARCH1  codep:0x224c wordp:0x42fa size:0x0008 C-string:'SEARCH1'
//         READNEX  codep:0x224c wordp:0x430e size:0x0008 C-string:'READNEX'
//         WRITENE  codep:0x224c wordp:0x4322 size:0x0008 C-string:'WRITENE'
//            MAKE  codep:0x224c wordp:0x4333 size:0x0008 C-string:'MAKE'
//         DOS-DTA  codep:0x224c wordp:0x4347 size:0x0014 C-string:'DOS_dash_DTA'
//         READ-RA  codep:0x224c wordp:0x4367 size:0x0008 C-string:'READ_dash_RA'
//         WRITE-R  codep:0x224c wordp:0x437b size:0x0008 C-string:'WRITE_dash_R'
//           W4385  codep:0x224c wordp:0x4385 size:0x000d C-string:'W4385'
//         MAKE<NA  codep:0x224c wordp:0x439e size:0x0018 C-string:'MAKE_st_NA'
//         OPEN<NA  codep:0x224c wordp:0x43c2 size:0x0016 C-string:'OPEN_st_NA'
//          [FILE]  codep:0x224c wordp:0x43e3 size:0x0026 C-string:'_bo_FILE_bc_'
//          SYSGEN  codep:0x224c wordp:0x4414 size:0x0030 C-string:'SYSGEN'
//         4TH>DOS  codep:0x224c wordp:0x4450 size:0x002e C-string:'_4TH_gt_DOS'
//           W4480  codep:0x224c wordp:0x4480 size:0x0056 C-string:'W4480'
//           W44D8  codep:0x224c wordp:0x44d8 size:0x0018 C-string:'W44D8'
//           W44F2  codep:0x224c wordp:0x44f2 size:0x000e C-string:'W44F2'
//         SETMAXD  codep:0x224c wordp:0x450c size:0x004c C-string:'SETMAXD'
//           W455A  codep:0x224c wordp:0x455a size:0x0012 C-string:'W455A'
//           ?1DRV  codep:0x224c wordp:0x4576 size:0x0016 C-string:'Is1DRV'
//           W458E  codep:0x224c wordp:0x458e size:0x0018 C-string:'W458E'
//         SMARTOP  codep:0x224c wordp:0x45b2 size:0x0050 C-string:'SMARTOP'
//         ASKMOUN  codep:0x1d29 wordp:0x460e size:0x0002 C-string:'ASKMOUN'
//            'BYE  codep:0x1d29 wordp:0x4619 size:0x0002 C-string:'_i_BYE'
//         SKIPPED  codep:0x1d29 wordp:0x4627 size:0x0002 C-string:'SKIPPED'
//         CANSKIP  codep:0x1d29 wordp:0x4635 size:0x0002 C-string:'CANSKIP'
//         <ASKMOU  codep:0x224c wordp:0x4643 size:0x008b C-string:'_st_ASKMOU'
//         DOSMOUN  codep:0x224c wordp:0x46da size:0x005e C-string:'DOSMOUN'
//         DOSUNMO  codep:0x224c wordp:0x4744 size:0x0010 C-string:'DOSUNMO'
//             DR2  codep:0x224c wordp:0x475c size:0x001e C-string:'DR2'
//             DR3  codep:0x224c wordp:0x4782 size:0x0020 C-string:'DR3'
//           USING  codep:0x224c wordp:0x47ac size:0x000e C-string:'USING'
//         SETBLOC  codep:0x224c wordp:0x47c6 size:0x0032 C-string:'SETBLOC'
//         SETSYSK  codep:0x224c wordp:0x4804 size:0x0048 C-string:'SETSYSK'
//           W484E  codep:0x1d29 wordp:0x484e size:0x0002 C-string:'W484E'
//           W4852  codep:0x1d29 wordp:0x4852 size:0x0002 C-string:'W4852'
//           W4856  codep:0x1d29 wordp:0x4856 size:0x0004 C-string:'W4856'
//            ?ERR  codep:0x224c wordp:0x4863 size:0x000a C-string:'IsERR'
//           W486F  codep:0x224c wordp:0x486f size:0x0032 C-string:'W486F'
//           [LDS]  codep:0x224c wordp:0x48ab size:0x0054 C-string:'_bo_LDS_bc_'
//           W4901  codep:0x1d29 wordp:0x4901 size:0x0011 C-string:'W4901'
//           (LDS)  codep:0x224c wordp:0x491c size:0x0036 C-string:'_ro_LDS_rc_'
//           W4954  codep:0x224c wordp:0x4954 size:0x0006 C-string:'W4954'
//            ON_1  codep:0x4961 wordp:0x4961 size:0x000b C-string:'ON_1'
//           OFF_2  codep:0x4974 wordp:0x4974 size:0x000a C-string:'OFF_2'
//           W4980  codep:0x4980 wordp:0x4980 size:0x0011 C-string:'W4980'
//           KEY_2  codep:0x224c wordp:0x4999 size:0x0006 C-string:'KEY_2'
//            NULL  codep:0x3b74 wordp:0x49a8 size:0x0004 C-string:'NULL'
//             ZZZ  codep:0x1d29 wordp:0x49b4 size:0x000a C-string:'ZZZ'
//         <LCMOVE  codep:0x49ca wordp:0x49ca size:0x002c C-string:'_st_LCMOVE'
//             2^N  codep:0x49fe wordp:0x49fe size:0x000f C-string:'_2_h_N'
//             BIT  codep:0x224c wordp:0x4a15 size:0x0006 C-string:'BIT'
//             16/  codep:0x4a23 wordp:0x4a23 size:0x000c C-string:'_16_slash_'
//             16*  codep:0x4a37 wordp:0x4a37 size:0x000c C-string:'_16_star_'
//            D16*  codep:0x4a4c wordp:0x4a4c size:0x0012 C-string:'D16_star_'
//              3*  codep:0x224c wordp:0x4a65 size:0x0006 C-string:'_3_star_'
//              3+  codep:0x4a72 wordp:0x4a72 size:0x000a C-string:'_3_plus_'
//            1.5@  codep:0x4a85 wordp:0x4a85 size:0x000e C-string:'_1_dot_5_at_'
//          1.5!_1  codep:0x4a9c wordp:0x4a9c size:0x000c C-string:'_1_dot_5_ex__1'
//             @DS  codep:0x4ab0 wordp:0x4ab0 size:0x0006 C-string:'GetDS'
//           W4AB8  codep:0x1d29 wordp:0x4ab8 size:0x0010 C-string:'W4AB8'
//              +@  codep:0x224c wordp:0x4acf size:0x0006 C-string:'_plus__at_'
//           >FLAG  codep:0x224c wordp:0x4adf size:0x0006 C-string:'_gt_FLAG'
//            L+-@  codep:0x4aee wordp:0x4aee size:0x0011 C-string:'L_plus__dash__at_'
//           W4B01  codep:0x4b01 wordp:0x4b01 size:0x0023 C-string:'W4B01'
//            CASE  codep:0x224c wordp:0x4b2d size:0x0017 C-string:'CASE'
//              IS  codep:0x224c wordp:0x4b4b size:0x000a C-string:'IS'
//          OTHERS  codep:0x224c wordp:0x4b60 size:0x0010 C-string:'OTHERS'
//           CASE:  codep:0x224c wordp:0x4b7a size:0x0019 C-string:'CASE_c_'
//          SEED_3  codep:0x1d29 wordp:0x4b9c size:0x0002 C-string:'SEED_3'
//            FRND  codep:0x4ba7 wordp:0x4ba7 size:0x0016 C-string:'FRND'
//            RRND  codep:0x224c wordp:0x4bc6 size:0x0010 C-string:'RRND'
//            +BIT  codep:0x4bdf wordp:0x4bdf size:0x0013 C-string:'_plus_BIT'
//             D2*  codep:0x4bf4 wordp:0x4bf4 size:0x000d C-string:'D2_star_'
//       EASY-BITS  codep:0x4c03 wordp:0x4c03 size:0x0025 C-string:'EASY_dash_BITS'
//           W4C2A  codep:0x224c wordp:0x4c2a size:0x0038 C-string:'W4C2A'
//           W4C64  codep:0x224c wordp:0x4c64 size:0x0026 C-string:'W4C64'
//            SQRT  codep:0x224c wordp:0x4c93 size:0x0016 C-string:'SQRT'
//             +-@  codep:0x4cb1 wordp:0x4cb1 size:0x000a C-string:'_plus__dash__at_'
//           L1.5@  codep:0x4cc5 wordp:0x4cc5 size:0x0015 C-string:'L1_dot_5_at_'
//            <C!>  codep:0x0c60 wordp:0x4ce3 size:0x0000 C-string:'_st_C_ex__gt_'
//             <!>  codep:0x0be1 wordp:0x4ceb size:0x0000 C-string:'_st__ex__gt_'
//          <1.5!>  codep:0x4a9c wordp:0x4cf6 size:0x0000 C-string:'_st_1_dot_5_ex__gt_'
//            <+!>  codep:0x0f85 wordp:0x4cff size:0x0000 C-string:'_st__plus__ex__gt_'
//            <D!>  codep:0x0c24 wordp:0x4d08 size:0x0000 C-string:'_st_D_ex__gt_'
//           <OFF>  codep:0x3af8 wordp:0x4d12 size:0x0002 C-string:'_st_OFF_gt_'
//            <ON>  codep:0x3af8 wordp:0x4d1d size:0x0002 C-string:'_st_ON_gt_'
//         <BLOCK>  codep:0x3af8 wordp:0x4d2b size:0x0002 C-string:'_st_BLOCK_gt_'
//         SPHEREW  codep:0x1d29 wordp:0x4d39 size:0x0002 C-string:'SPHEREW'
//         SIGNEXT  codep:0x1d29 wordp:0x4d47 size:0x0002 C-string:'SIGNEXT'
//           W4D4B  codep:0x1d29 wordp:0x4d4b size:0x0002 C-string:'W4D4B'
//           W4D4F  codep:0x1d29 wordp:0x4d4f size:0x0002 C-string:'W4D4F'
//           W4D53  codep:0x1d29 wordp:0x4d53 size:0x0002 C-string:'W4D53'
//          'ARRAY  codep:0x1d29 wordp:0x4d60 size:0x0002 C-string:'_i_ARRAY'
//         ARRAYSE  codep:0x224c wordp:0x4d6e size:0x000a C-string:'ARRAYSE'
//          #BYTES  codep:0x224c wordp:0x4d83 size:0x000a C-string:'_n_BYTES'
//           #COLZ  codep:0x224c wordp:0x4d97 size:0x0008 C-string:'_n_COLZ'
//           #ROWZ  codep:0x224c wordp:0x4da9 size:0x000a C-string:'_n_ROWZ'
//         ACELLAD  codep:0x224c wordp:0x4dbf size:0x000a C-string:'ACELLAD'
//           W4DCB  codep:0x4dcb wordp:0x4dcb size:0x0022 C-string:'W4DCB'
//              A!  codep:0x224c wordp:0x4df4 size:0x0006 C-string:'A_ex_'
//              A@  codep:0x224c wordp:0x4e01 size:0x0006 C-string:'A_at_'
//        !OFFSETS  codep:0x4e13 wordp:0x4e13 size:0x002c C-string:'StoreOFFSETS'
//           ARRAY  codep:0x224c wordp:0x4e49 size:0x002d C-string:'ARRAY'
//         SETLARR  codep:0x224c wordp:0x4e82 size:0x002e C-string:'SETLARR'
//             XLL  codep:0x1d29 wordp:0x4eb8 size:0x0002 C-string:'XLL'
//             YLL  codep:0x1d29 wordp:0x4ec2 size:0x0002 C-string:'YLL'
//             XUR  codep:0x1d29 wordp:0x4ecc size:0x0002 C-string:'XUR'
//             YUR  codep:0x1d29 wordp:0x4ed6 size:0x0002 C-string:'YUR'
//         FULLARR  codep:0x224c wordp:0x4ee4 size:0x000e C-string:'FULLARR'
//         SETREGI  codep:0x224c wordp:0x4efe size:0x0012 C-string:'SETREGI'
//         FILLREG  codep:0x224c wordp:0x4f1c size:0x002e C-string:'FILLREG'
//           FONT#  codep:0x1d29 wordp:0x4f54 size:0x0002 C-string:'FONT_n_'
//           TANDY  codep:0x1d29 wordp:0x4f60 size:0x0002 C-string:'TANDY'
//          COLORT  codep:0x1d29 wordp:0x4f6d size:0x0011 C-string:'COLORT'
//             I>C  codep:0x224c wordp:0x4f86 size:0x0008 C-string:'I_gt_C'
//           W4F90  codep:0x224c wordp:0x4f90 size:0x000f C-string:'W4F90'
//           BLACK  codep:0x4f96 wordp:0x4fa9 size:0x0001 C-string:'BLACK'
//         DK-BLUE  codep:0x4f96 wordp:0x4fb6 size:0x0001 C-string:'DK_dash_BLUE'
//         DK-GREE  codep:0x4f96 wordp:0x4fc3 size:0x0001 C-string:'DK_dash_GREE'
//           GREEN  codep:0x4f96 wordp:0x4fce size:0x0001 C-string:'GREEN'
//             RED  codep:0x4f96 wordp:0x4fd7 size:0x0001 C-string:'RED'
//          VIOLET  codep:0x4f96 wordp:0x4fe3 size:0x0001 C-string:'VIOLET'
//           BROWN  codep:0x4f96 wordp:0x4fee size:0x0001 C-string:'BROWN'
//           GREY1  codep:0x4f96 wordp:0x4ff9 size:0x0001 C-string:'GREY1'
//           GREY2  codep:0x4f96 wordp:0x5004 size:0x0001 C-string:'GREY2'
//            BLUE  codep:0x4f96 wordp:0x500e size:0x0001 C-string:'BLUE'
//         LT-GREE  codep:0x4f96 wordp:0x501b size:0x0001 C-string:'LT_dash_GREE'
//         LT-BLUE  codep:0x4f96 wordp:0x5028 size:0x0001 C-string:'LT_dash_BLUE'
//            PINK  codep:0x4f96 wordp:0x5032 size:0x0001 C-string:'PINK'
//          ORANGE  codep:0x4f96 wordp:0x503e size:0x0001 C-string:'ORANGE'
//          YELLOW  codep:0x4f96 wordp:0x504a size:0x0001 C-string:'YELLOW'
//           WHITE  codep:0x4f96 wordp:0x5055 size:0x0001 C-string:'WHITE'
//            ?NID  codep:0x1d29 wordp:0x505f size:0x0002 C-string:'IsNID'
//            BMAP  codep:0x1d29 wordp:0x506a size:0x000e C-string:'BMAP'
//           W507A  codep:0x224c wordp:0x507a size:0x0006 C-string:'W507A'
//           BMOFF  codep:0x224c wordp:0x508a size:0x0006 C-string:'BMOFF'
//           BMSEG  codep:0x224c wordp:0x509a size:0x0006 C-string:'BMSEG'
//          BMWIDE  codep:0x224c wordp:0x50ab size:0x0006 C-string:'BMWIDE'
//         BMBYTES  codep:0x224c wordp:0x50bd size:0x0008 C-string:'BMBYTES'
//          BMHIGH  codep:0x224c wordp:0x50d0 size:0x0008 C-string:'BMHIGH'
//           BMPAL  codep:0x224c wordp:0x50e2 size:0x0008 C-string:'BMPAL'
//         IHEADLE  codep:0x2214 wordp:0x50f6 size:0x0002 C-string:'IHEADLE'
//           W50FA  codep:0x2214 wordp:0x50fa size:0x0002 C-string:'W50FA'
//           W50FE  codep:0x2214 wordp:0x50fe size:0x0002 C-string:'W50FE'
//           W5102  codep:0x2214 wordp:0x5102 size:0x0002 C-string:'W5102'
//         *MAPSCA  codep:0x2214 wordp:0x5110 size:0x0002 C-string:'_star_MAPSCA'
//          SIGBLK  codep:0x2214 wordp:0x511d size:0x0002 C-string:'SIGBLK'
//          ALOVSA  codep:0x2214 wordp:0x512a size:0x0002 C-string:'ALOVSA'
//          AHIVSA  codep:0x2214 wordp:0x5137 size:0x0002 C-string:'AHIVSA'
//          BLOVSA  codep:0x2214 wordp:0x5144 size:0x0002 C-string:'BLOVSA'
//          BHIVSA  codep:0x2214 wordp:0x5151 size:0x0002 C-string:'BHIVSA'
//         'OVBACK  codep:0x2214 wordp:0x515f size:0x0002 C-string:'_i_OVBACK'
//          MUSSEG  codep:0x2214 wordp:0x516c size:0x0002 C-string:'MUSSEG'
//             WIN  codep:0x1d29 wordp:0x5176 size:0x0002 C-string:'WIN'
//         ?FIRED-  codep:0x2214 wordp:0x5184 size:0x0002 C-string:'IsFIRED_dash_'
//         ?A-SHIE  codep:0x2214 wordp:0x5192 size:0x0002 C-string:'IsA_dash_SHIE'
//         ?A-WEAP  codep:0x2214 wordp:0x51a0 size:0x0002 C-string:'IsA_dash_WEAP'
//             16K  codep:0x2214 wordp:0x51aa size:0x0002 C-string:'_16K'
//         DBUF-SI  codep:0x3b74 wordp:0x51b8 size:0x0004 C-string:'DBUF_dash_SI'
//            TRUE  codep:0x2214 wordp:0x51c5 size:0x0002 C-string:'TRUE'
//           FALSE  codep:0x2214 wordp:0x51d1 size:0x0002 C-string:'FALSE'
//             ?UF  codep:0x1d29 wordp:0x51db size:0x0002 C-string:'IsUF'
//             ?AF  codep:0x1d29 wordp:0x51e5 size:0x0002 C-string:'IsAF'
//          #SHOTS  codep:0x1d29 wordp:0x51f2 size:0x0002 C-string:'_n_SHOTS'
//           'ASYS  codep:0x1d29 wordp:0x51fe size:0x0002 C-string:'_i_ASYS'
//             P/B  codep:0x1d29 wordp:0x5208 size:0x0002 C-string:'P_slash_B'
//           'BOSS  codep:0x1d29 wordp:0x5214 size:0x0002 C-string:'_i_BOSS'
//           %TALK  codep:0x1d29 wordp:0x5220 size:0x0002 C-string:'_pe_TALK'
//         NULL-IC  codep:0x2214 wordp:0x522e size:0x0002 C-string:'NULL_dash_IC'
//         SYS-ICO  codep:0x2214 wordp:0x523c size:0x0002 C-string:'SYS_dash_ICO'
//         INVIS-I  codep:0x2214 wordp:0x524a size:0x0002 C-string:'INVIS_dash_I'
//         FLUX-IC  codep:0x2214 wordp:0x5258 size:0x0002 C-string:'FLUX_dash_IC'
//         DEAD-IC  codep:0x2214 wordp:0x5266 size:0x0002 C-string:'DEAD_dash_IC'
//         DEFAULT  codep:0x2214 wordp:0x5274 size:0x0002 C-string:'DEFAULT'
//         TEXTC/L  codep:0x2214 wordp:0x5282 size:0x0002 C-string:'TEXTC_slash_L'
//           W5286  codep:0x2214 wordp:0x5286 size:0x0002 C-string:'W5286'
//         FUEL/SE  codep:0x2214 wordp:0x5294 size:0x0002 C-string:'FUEL_slash_SE'
//         POLYSEG  codep:0x2214 wordp:0x52a2 size:0x0002 C-string:'POLYSEG'
//          'ANSYS  codep:0x2214 wordp:0x52af size:0x0002 C-string:'_i_ANSYS'
//         ?CALLED  codep:0x2214 wordp:0x52bd size:0x0002 C-string:'IsCALLED'
//              0.  codep:0x3b74 wordp:0x52c6 size:0x0004 C-string:'_0_dot_'
//         VANEWSP  codep:0x3b74 wordp:0x52d6 size:0x0004 C-string:'VANEWSP'
//           IROOT  codep:0x3b74 wordp:0x52e4 size:0x0004 C-string:'IROOT'
//         INACTIV  codep:0x3b74 wordp:0x52f4 size:0x0004 C-string:'INACTIV'
//         FRAGMEN  codep:0x3b74 wordp:0x5304 size:0x0004 C-string:'FRAGMEN'
//           *SECS  codep:0x3b74 wordp:0x5312 size:0x0004 C-string:'_star_SECS'
//         *ASSIGN  codep:0x3b74 wordp:0x5322 size:0x0004 C-string:'_star_ASSIGN'
//           *SHIP  codep:0x3b74 wordp:0x5330 size:0x0004 C-string:'_star_SHIP'
//           *ASYS  codep:0x3b74 wordp:0x533e size:0x0004 C-string:'_star_ASYS'
//           *ARTH  codep:0x3b74 wordp:0x534c size:0x0004 C-string:'_star_ARTH'
//         *ARREST  codep:0x3b74 wordp:0x535c size:0x0004 C-string:'_star_ARREST'
//             *HL  codep:0x3b74 wordp:0x5368 size:0x0004 C-string:'_star_HL'
//             *LH  codep:0x3b74 wordp:0x5374 size:0x0004 C-string:'_star_LH'
//             *GD  codep:0x3b74 wordp:0x5380 size:0x0004 C-string:'_star_GD'
//          *4SALE  codep:0x3b74 wordp:0x538f size:0x0004 C-string:'_star_4SALE'
//           *2BUY  codep:0x3b74 wordp:0x539d size:0x0004 C-string:'_star_2BUY'
//             MPS  codep:0x2214 wordp:0x53a9 size:0x0002 C-string:'MPS'
//         BOX-IAD  codep:0x3b74 wordp:0x53b7 size:0x0004 C-string:'BOX_dash_IAD'
//           *DICT  codep:0x3b74 wordp:0x53c5 size:0x0004 C-string:'_star_DICT'
//         CLIP-TA  codep:0x1d29 wordp:0x53d5 size:0x0014 C-string:'CLIP_dash_TA'
//           TABLE  codep:0x224c wordp:0x53f3 size:0x0011 C-string:'TABLE'
//         FONTSEG  codep:0x1d29 wordp:0x5410 size:0x0002 C-string:'FONTSEG'
//          REMSEG  codep:0x1d29 wordp:0x541d size:0x0002 C-string:'REMSEG'
//         XBUF-SE  codep:0x1d29 wordp:0x542b size:0x0002 C-string:'XBUF_dash_SE'
//            DICT  codep:0x1d29 wordp:0x5436 size:0x0002 C-string:'DICT'
//            ?ALL  codep:0x1d29 wordp:0x5441 size:0x0002 C-string:'IsALL'
//            ?YOK  codep:0x1d29 wordp:0x544c size:0x0002 C-string:'IsYOK'
//             HUB  codep:0x1d29 wordp:0x5456 size:0x0002 C-string:'HUB'
//            PIC#  codep:0x1d29 wordp:0x5461 size:0x0002 C-string:'PIC_n_'
//           1LOGO  codep:0x1d29 wordp:0x546d size:0x000e C-string:'_1LOGO'
//           W547D  codep:0x1d29 wordp:0x547d size:0x000a C-string:'W547D'
//           W5489  codep:0x1d29 wordp:0x5489 size:0x0006 C-string:'W5489'
//           W5491  codep:0x1d29 wordp:0x5491 size:0x0002 C-string:'W5491'
//          'XCOMM  codep:0x1d29 wordp:0x549e size:0x0002 C-string:'_i_XCOMM'
//            TCLR  codep:0x1d29 wordp:0x54a9 size:0x0002 C-string:'TCLR'
//           'TRAK  codep:0x1d29 wordp:0x54b5 size:0x0002 C-string:'_i_TRAK'
//         TRAK-HR  codep:0x1d29 wordp:0x54c3 size:0x0002 C-string:'TRAK_dash_HR'
//         A-STREN  codep:0x1d29 wordp:0x54d1 size:0x0002 C-string:'A_dash_STREN'
//          DIRBLK  codep:0x1d29 wordp:0x54de size:0x0002 C-string:'DIRBLK'
//         TIMESTA  codep:0x1d29 wordp:0x54ec size:0x0002 C-string:'TIMESTA'
//         RELAXTI  codep:0x1d29 wordp:0x54fa size:0x0000 C-string:'RELAXTI'
//           W54FC  codep:0x7420 wordp:0x54fc size:0x0000 C-string:'W54FC'
//         ):-,601  codep:0x1d29 wordp:0x5508 size:0x0004 C-string:'_rc__c__dash__co_601'
//         ICONFON  codep:0x1d29 wordp:0x5518 size:0x0002 C-string:'ICONFON'
//          LFRAME  codep:0x1d29 wordp:0x5525 size:0x0002 C-string:'LFRAME'
//           ?AUTO  codep:0x1d29 wordp:0x5531 size:0x0002 C-string:'IsAUTO'
//         ?CRITIC  codep:0x1d29 wordp:0x553f size:0x0002 C-string:'IsCRITIC'
//         P-POSTU  codep:0x1d29 wordp:0x554d size:0x0002 C-string:'P_dash_POSTU'
//         ELEM-AM  codep:0x1d29 wordp:0x555b size:0x0002 C-string:'ELEM_dash_AM'
//         'INJURE  codep:0x1d29 wordp:0x5569 size:0x0002 C-string:'_i_INJURE'
//         P-COLOR  codep:0x1d29 wordp:0x5577 size:0x0003 C-string:'P_dash_COLOR'
//         P-PHRAS  codep:0x1d29 wordp:0x5586 size:0x0003 C-string:'P_dash_PHRAS'
//          P-CARP  codep:0x1d29 wordp:0x5594 size:0x0003 C-string:'P_dash_CARP'
//         O-COLOR  codep:0x1d29 wordp:0x55a3 size:0x0003 C-string:'O_dash_COLOR'
//           HYDRO  codep:0x1d29 wordp:0x55b0 size:0x0002 C-string:'HYDRO'
//            ATMO  codep:0x1d29 wordp:0x55bb size:0x0002 C-string:'ATMO'
//          LCOLOR  codep:0x1d29 wordp:0x55c8 size:0x0002 C-string:'LCOLOR'
//             ?TV  codep:0x1d29 wordp:0x55d2 size:0x0002 C-string:'IsTV'
//           OLDHR  codep:0x1d29 wordp:0x55de size:0x0002 C-string:'OLDHR'
//           AVCNT  codep:0x1d29 wordp:0x55ea size:0x0002 C-string:'AVCNT'
//           W55EE  codep:0x1d29 wordp:0x55ee size:0x0002 C-string:'W55EE'
//           ?BOMB  codep:0x1d29 wordp:0x55fa size:0x0003 C-string:'IsBOMB'
//           W55FF  codep:0x1d29 wordp:0x55ff size:0x0002 C-string:'W55FF'
//           W5603  codep:0x1d29 wordp:0x5603 size:0x0002 C-string:'W5603'
//           W5607  codep:0x1d29 wordp:0x5607 size:0x0002 C-string:'W5607'
//           FILE#  codep:0x1d29 wordp:0x5613 size:0x0002 C-string:'FILE_n_'
//         RECORD#  codep:0x1d29 wordp:0x5621 size:0x0002 C-string:'RECORD_n_'
//           W5625  codep:0x1d29 wordp:0x5625 size:0x0002 C-string:'W5625'
//           W5629  codep:0x1d29 wordp:0x5629 size:0x0002 C-string:'W5629'
//            CXSP  codep:0x1d29 wordp:0x5634 size:0x0002 C-string:'CXSP'
//           W5638  codep:0x1d29 wordp:0x5638 size:0x0002 C-string:'W5638'
//           W563C  codep:0x1d29 wordp:0x563c size:0x0002 C-string:'W563C'
//             EDL  codep:0x1d29 wordp:0x5646 size:0x0002 C-string:'EDL'
//         A-POSTU  codep:0x1d29 wordp:0x5654 size:0x0002 C-string:'A_dash_POSTU'
//           W5658  codep:0x1d29 wordp:0x5658 size:0x0002 C-string:'W5658'
//         P-RACES  codep:0x1d29 wordp:0x5666 size:0x0002 C-string:'P_dash_RACES'
//           W566A  codep:0x1d29 wordp:0x566a size:0x0002 C-string:'W566A'
//           W566E  codep:0x1d29 wordp:0x566e size:0x0002 C-string:'W566E'
//           W5672  codep:0x1d29 wordp:0x5672 size:0x0002 C-string:'W5672'
//           W5676  codep:0x1d29 wordp:0x5676 size:0x0002 C-string:'W5676'
//           W567A  codep:0x1d29 wordp:0x567a size:0x0002 C-string:'W567A'
//         'THROW-  codep:0x1d29 wordp:0x5688 size:0x0002 C-string:'_i_THROW_dash_'
//            'MAP  codep:0x1d29 wordp:0x5693 size:0x0002 C-string:'_i_MAP'
//         'TRAVER  codep:0x1d29 wordp:0x56a1 size:0x0002 C-string:'_i_TRAVER'
//          '?EXIT  codep:0x1d29 wordp:0x56ae size:0x0002 C-string:'_i__ask_EXIT'
//           W56B2  codep:0x1d29 wordp:0x56b2 size:0x0002 C-string:'W56B2'
//         '.FLUX-  codep:0x1d29 wordp:0x56c0 size:0x0002 C-string:'_i__dot_FLUX_dash_'
//             ?TD  codep:0x1d29 wordp:0x56ca size:0x0002 C-string:'IsTD'
//         ?LANDED  codep:0x1d29 wordp:0x56d8 size:0x0002 C-string:'IsLANDED'
//           W56DC  codep:0x1d29 wordp:0x56dc size:0x0002 C-string:'W56DC'
//           W56E0  codep:0x1d29 wordp:0x56e0 size:0x0002 C-string:'W56E0'
//           W56E4  codep:0x1d29 wordp:0x56e4 size:0x0002 C-string:'W56E4'
//           W56E8  codep:0x1d29 wordp:0x56e8 size:0x0002 C-string:'W56E8'
//           W56EC  codep:0x1d29 wordp:0x56ec size:0x0002 C-string:'W56EC'
//           W56F0  codep:0x1d29 wordp:0x56f0 size:0x0002 C-string:'W56F0'
//           W56F4  codep:0x1d29 wordp:0x56f4 size:0x0002 C-string:'W56F4'
//           W56F8  codep:0x1d29 wordp:0x56f8 size:0x0002 C-string:'W56F8'
//            -END  codep:0x1d29 wordp:0x5703 size:0x0002 C-string:'_dash_END'
//             OV#  codep:0x1d29 wordp:0x570d size:0x0002 C-string:'OV_n_'
//           W5711  codep:0x1d29 wordp:0x5711 size:0x0002 C-string:'W5711'
//         REPAIRT  codep:0x1d29 wordp:0x571f size:0x0002 C-string:'REPAIRT'
//         HBUF-SE  codep:0x1d29 wordp:0x572d size:0x0002 C-string:'HBUF_dash_SE'
//         DBUF-SE  codep:0x1d29 wordp:0x573b size:0x0002 C-string:'DBUF_dash_SE'
//           COLOR  codep:0x1d29 wordp:0x5747 size:0x0002 C-string:'COLOR'
//          DCOLOR  codep:0x1d29 wordp:0x5754 size:0x0002 C-string:'DCOLOR'
//            YTAB  codep:0x1d29 wordp:0x575f size:0x0002 C-string:'YTAB'
//              Y1  codep:0x1d29 wordp:0x5768 size:0x0002 C-string:'Y1'
//              X1  codep:0x1d29 wordp:0x5771 size:0x0002 C-string:'X1'
//              Y2  codep:0x1d29 wordp:0x577a size:0x0002 C-string:'Y2'
//              X2  codep:0x1d29 wordp:0x5783 size:0x0002 C-string:'X2'
//           YTABL  codep:0x1d29 wordp:0x578f size:0x0002 C-string:'YTABL'
//         BUF-SEG  codep:0x1d29 wordp:0x579d size:0x0002 C-string:'BUF_dash_SEG'
//          RETURN  codep:0x1d29 wordp:0x57aa size:0x0002 C-string:'RETURN'
//            ?WIN  codep:0x1d29 wordp:0x57b5 size:0x0002 C-string:'IsWIN'
//             #IN  codep:0x1d29 wordp:0x57bf size:0x0002 C-string:'_n_IN'
//            #OUT  codep:0x1d29 wordp:0x57ca size:0x0002 C-string:'_n_OUT'
//             VIN  codep:0x1d29 wordp:0x57d4 size:0x0002 C-string:'VIN'
//            VOUT  codep:0x1d29 wordp:0x57df size:0x0002 C-string:'VOUT'
//             OIN  codep:0x1d29 wordp:0x57e9 size:0x0002 C-string:'OIN'
//            OOUT  codep:0x1d29 wordp:0x57f4 size:0x0002 C-string:'OOUT'
//           IVPTR  codep:0x1d29 wordp:0x5800 size:0x0002 C-string:'IVPTR'
//           OVPTR  codep:0x1d29 wordp:0x580c size:0x0002 C-string:'OVPTR'
//           ICPTR  codep:0x1d29 wordp:0x5818 size:0x0002 C-string:'ICPTR'
//           OCPTR  codep:0x1d29 wordp:0x5824 size:0x0002 C-string:'OCPTR'
//            FLIP  codep:0x1d29 wordp:0x582f size:0x0002 C-string:'FLIP'
//          TACCPT  codep:0x1d29 wordp:0x583c size:0x0002 C-string:'TACCPT'
//           TRJCT  codep:0x1d29 wordp:0x5848 size:0x0002 C-string:'TRJCT'
//           ?OPEN  codep:0x1d29 wordp:0x5854 size:0x0002 C-string:'IsOPEN'
//           ?EVAL  codep:0x1d29 wordp:0x5860 size:0x0002 C-string:'IsEVAL'
//              SX  codep:0x1d29 wordp:0x5869 size:0x0002 C-string:'SX'
//              SY  codep:0x1d29 wordp:0x5872 size:0x0002 C-string:'SY'
//              SO  codep:0x1d29 wordp:0x587b size:0x0002 C-string:'SO'
//              PX  codep:0x1d29 wordp:0x5884 size:0x0002 C-string:'PX'
//              PY  codep:0x1d29 wordp:0x588d size:0x0002 C-string:'PY'
//              PO  codep:0x1d29 wordp:0x5896 size:0x0002 C-string:'PO'
//              IX  codep:0x1d29 wordp:0x589f size:0x0002 C-string:'IX'
//              IY  codep:0x1d29 wordp:0x58a8 size:0x0002 C-string:'IY'
//              IO  codep:0x1d29 wordp:0x58b1 size:0x0002 C-string:'IO'
//         COMPARE  codep:0x1d29 wordp:0x58bf size:0x0002 C-string:'COMPARE'
//         X-INTER  codep:0x1d29 wordp:0x58cd size:0x0002 C-string:'X_dash_INTER'
//           ILEFT  codep:0x1d29 wordp:0x58d9 size:0x0002 C-string:'ILEFT'
//          IRIGHT  codep:0x1d29 wordp:0x58e6 size:0x0002 C-string:'IRIGHT'
//          IBELOW  codep:0x1d29 wordp:0x58f3 size:0x0002 C-string:'IBELOW'
//          IABOVE  codep:0x1d29 wordp:0x5900 size:0x0002 C-string:'IABOVE'
//            'FLY  codep:0x1d29 wordp:0x590b size:0x0002 C-string:'_i_FLY'
//         'UNNEST  codep:0x1d29 wordp:0x5919 size:0x0002 C-string:'_i_UNNEST'
//            ?NEW  codep:0x1d29 wordp:0x5924 size:0x0002 C-string:'IsNEW'
//          FORCED  codep:0x1d29 wordp:0x5931 size:0x0002 C-string:'FORCED'
//           #VESS  codep:0x1d29 wordp:0x593d size:0x0002 C-string:'_n_VESS'
//         CTCOLOR  codep:0x1d29 wordp:0x594b size:0x0002 C-string:'CTCOLOR'
//          XSTART  codep:0x1d29 wordp:0x5958 size:0x0002 C-string:'XSTART'
//            XEND  codep:0x1d29 wordp:0x5963 size:0x0002 C-string:'XEND'
//           YLINE  codep:0x1d29 wordp:0x596f size:0x0002 C-string:'YLINE'
//              ?3  codep:0x1d29 wordp:0x5978 size:0x0002 C-string:'Is3'
//          '.HUFF  codep:0x1d29 wordp:0x5985 size:0x0002 C-string:'_i__dot_HUFF'
//            SCAN  codep:0x1d29 wordp:0x5990 size:0x0002 C-string:'SCAN'
//            LMNT  codep:0x1d29 wordp:0x599b size:0x0002 C-string:'LMNT'
//           SCAN+  codep:0x1d29 wordp:0x59a7 size:0x0002 C-string:'SCAN_plus_'
//            YMIN  codep:0x1d29 wordp:0x59b2 size:0x0002 C-string:'YMIN'
//            YMAX  codep:0x1d29 wordp:0x59bd size:0x0002 C-string:'YMAX'
//          #HORIZ  codep:0x1d29 wordp:0x59ca size:0x0002 C-string:'_n_HORIZ'
//           MOVED  codep:0x1d29 wordp:0x59d6 size:0x0002 C-string:'MOVED'
//          MEMSEG  codep:0x1d29 wordp:0x59e3 size:0x0002 C-string:'MEMSEG'
//          MEMOFF  codep:0x1d29 wordp:0x59f0 size:0x0002 C-string:'MEMOFF'
//         MONITOR  codep:0x1d29 wordp:0x59fe size:0x0002 C-string:'MONITOR'
//         LOCRADI  codep:0x1d29 wordp:0x5a0c size:0x0002 C-string:'LOCRADI'
//         (ELIST)  codep:0x1d29 wordp:0x5a1a size:0x0003 C-string:'_ro_ELIST_rc_'
//           ICON^  codep:0x1d29 wordp:0x5a27 size:0x0002 C-string:'ICON_h_'
//            PAL^  codep:0x1d29 wordp:0x5a32 size:0x0002 C-string:'PAL_h_'
//            YBLT  codep:0x1d29 wordp:0x5a3d size:0x0002 C-string:'YBLT'
//            XBLT  codep:0x1d29 wordp:0x5a48 size:0x0002 C-string:'XBLT'
//         XORMODE  codep:0x1d29 wordp:0x5a56 size:0x0002 C-string:'XORMODE'
//            LBLT  codep:0x1d29 wordp:0x5a61 size:0x0002 C-string:'LBLT'
//            WBLT  codep:0x1d29 wordp:0x5a6c size:0x0002 C-string:'WBLT'
//            ABLT  codep:0x1d29 wordp:0x5a77 size:0x0002 C-string:'ABLT'
//          BLTSEG  codep:0x1d29 wordp:0x5a84 size:0x0002 C-string:'BLTSEG'
//            BLT>  codep:0x1d29 wordp:0x5a8f size:0x0002 C-string:'BLT_gt_'
//         TILE-PT  codep:0x1d29 wordp:0x5a9d size:0x0002 C-string:'TILE_dash_PT'
//         ?FUEL-D  codep:0x1d29 wordp:0x5aab size:0x0002 C-string:'IsFUEL_dash_D'
//         ?G-AWAR  codep:0x1d29 wordp:0x5ab9 size:0x0002 C-string:'IsG_dash_AWAR'
//             GWF  codep:0x1d29 wordp:0x5ac3 size:0x0002 C-string:'GWF'
//           DXVIS  codep:0x1d29 wordp:0x5acf size:0x0002 C-string:'DXVIS'
//           DYVIS  codep:0x1d29 wordp:0x5adb size:0x0002 C-string:'DYVIS'
//            XCON  codep:0x1d29 wordp:0x5ae6 size:0x0002 C-string:'XCON'
//            YCON  codep:0x1d29 wordp:0x5af1 size:0x0002 C-string:'YCON'
//           DXCON  codep:0x1d29 wordp:0x5afd size:0x0002 C-string:'DXCON'
//           DYCON  codep:0x1d29 wordp:0x5b09 size:0x0002 C-string:'DYCON'
//            XVIS  codep:0x1d29 wordp:0x5b14 size:0x0002 C-string:'XVIS'
//            YVIS  codep:0x1d29 wordp:0x5b1f size:0x0002 C-string:'YVIS'
//         XLLDEST  codep:0x1d29 wordp:0x5b2d size:0x0002 C-string:'XLLDEST'
//         YLLDEST  codep:0x1d29 wordp:0x5b3b size:0x0002 C-string:'YLLDEST'
//         GLOBALS  codep:0x1d29 wordp:0x5b49 size:0x0002 C-string:'GLOBALS'
//          '.CELL  codep:0x1d29 wordp:0x5b56 size:0x0002 C-string:'_i__dot_CELL'
//         '.BACKG  codep:0x1d29 wordp:0x5b64 size:0x0002 C-string:'_i__dot_BACKG'
//         'ICON-P  codep:0x1d29 wordp:0x5b72 size:0x0002 C-string:'_i_ICON_dash_P'
//         'ICONBO  codep:0x1d29 wordp:0x5b80 size:0x0002 C-string:'_i_ICONBO'
//             'CC  codep:0x1d29 wordp:0x5b8a size:0x0002 C-string:'_i_CC'
//           W5B8E  codep:0x1d29 wordp:0x5b8e size:0x0002 C-string:'W5B8E'
//           W5B92  codep:0x1d29 wordp:0x5b92 size:0x0002 C-string:'W5B92'
//           W5B96  codep:0x1d29 wordp:0x5b96 size:0x0002 C-string:'W5B96'
//           W5B9A  codep:0x1d29 wordp:0x5b9a size:0x0002 C-string:'W5B9A'
//           W5B9E  codep:0x1d29 wordp:0x5b9e size:0x0002 C-string:'W5B9E'
//           IHSEG  codep:0x1d29 wordp:0x5baa size:0x0002 C-string:'IHSEG'
//         IGLOBAL  codep:0x1d29 wordp:0x5bb8 size:0x0002 C-string:'IGLOBAL'
//          ILOCAL  codep:0x1d29 wordp:0x5bc5 size:0x0002 C-string:'ILOCAL'
//          IINDEX  codep:0x1d29 wordp:0x5bd2 size:0x0002 C-string:'IINDEX'
//            XWLL  codep:0x1d29 wordp:0x5bdd size:0x0002 C-string:'XWLL'
//            YWLL  codep:0x1d29 wordp:0x5be8 size:0x0000 C-string:'YWLL'
//           W5BEA  codep:0x7420 wordp:0x5bea size:0x0000 C-string:'W5BEA'
//            XWUR  codep:0x1d29 wordp:0x5bf3 size:0x0002 C-string:'XWUR'
//            YWUR  codep:0x1d29 wordp:0x5bfe size:0x0002 C-string:'YWUR'
//         *GLOBAL  codep:0x1d29 wordp:0x5c0c size:0x0002 C-string:'_star_GLOBAL'
//         (STOP-C  codep:0x1d29 wordp:0x5c1a size:0x0002 C-string:'_ro_STOP_dash_C'
//           W5C1E  codep:0x1d29 wordp:0x5c1e size:0x0002 C-string:'W5C1E'
//       CONTEXT_3  codep:0x1d29 wordp:0x5c2c size:0x0002 C-string:'CONTEXT_3'
//            %EFF  codep:0x1d29 wordp:0x5c37 size:0x0002 C-string:'_pe_EFF'
//           STORM  codep:0x1d29 wordp:0x5c43 size:0x0002 C-string:'STORM'
//            'TVT  codep:0x1d29 wordp:0x5c4e size:0x0002 C-string:'_i_TVT'
//          'STORM  codep:0x1d29 wordp:0x5c5b size:0x0002 C-string:'_i_STORM'
//             E/M  codep:0x1d29 wordp:0x5c65 size:0x0002 C-string:'E_slash_M'
//         FORCEPT  codep:0x1d29 wordp:0x5c73 size:0x0002 C-string:'FORCEPT'
//          #STORM  codep:0x1d29 wordp:0x5c80 size:0x0002 C-string:'_n_STORM'
//           W5C84  codep:0x1d29 wordp:0x5c84 size:0x0002 C-string:'W5C84'
//           W5C88  codep:0x1d29 wordp:0x5c88 size:0x0002 C-string:'W5C88'
//           W5C8C  codep:0x1d29 wordp:0x5c8c size:0x0002 C-string:'W5C8C'
//           W5C90  codep:0x1d29 wordp:0x5c90 size:0x0002 C-string:'W5C90'
//         PORTDAT  codep:0x1d29 wordp:0x5c9e size:0x0002 C-string:'PORTDAT'
//           ?PORT  codep:0x1d29 wordp:0x5caa size:0x0002 C-string:'IsPORT'
//            TVIS  codep:0x1d29 wordp:0x5cb5 size:0x0002 C-string:'TVIS'
//            RVIS  codep:0x1d29 wordp:0x5cc0 size:0x0002 C-string:'RVIS'
//            BVIS  codep:0x1d29 wordp:0x5ccb size:0x0002 C-string:'BVIS'
//            LVIS  codep:0x1d29 wordp:0x5cd6 size:0x0002 C-string:'LVIS'
//           LFSEG  codep:0x1d29 wordp:0x5ce2 size:0x0002 C-string:'LFSEG'
//          LSYSEG  codep:0x1d29 wordp:0x5cef size:0x0002 C-string:'LSYSEG'
//          MSYSEG  codep:0x1d29 wordp:0x5cfc size:0x0002 C-string:'MSYSEG'
//          SSYSEG  codep:0x1d29 wordp:0x5d09 size:0x0002 C-string:'SSYSEG'
//         ?REPAIR  codep:0x1d29 wordp:0x5d17 size:0x0002 C-string:'IsREPAIR'
//           ?HEAL  codep:0x1d29 wordp:0x5d23 size:0x0002 C-string:'IsHEAL'
//           MXNEB  codep:0x1d29 wordp:0x5d2f size:0x0002 C-string:'MXNEB'
//         THIS-BU  codep:0x1d29 wordp:0x5d3d size:0x0002 C-string:'THIS_dash_BU'
//            NCRS  codep:0x1d29 wordp:0x5d48 size:0x0002 C-string:'NCRS'
//            OCRS  codep:0x1d29 wordp:0x5d53 size:0x0002 C-string:'OCRS'
//            WTOP  codep:0x1d29 wordp:0x5d5e size:0x0002 C-string:'WTOP'
//         WBOTTOM  codep:0x1d29 wordp:0x5d6c size:0x0002 C-string:'WBOTTOM'
//          WRIGHT  codep:0x1d29 wordp:0x5d79 size:0x0002 C-string:'WRIGHT'
//           WLEFT  codep:0x1d29 wordp:0x5d85 size:0x0002 C-string:'WLEFT'
//          WLINES  codep:0x1d29 wordp:0x5d92 size:0x0002 C-string:'WLINES'
//          WCHARS  codep:0x1d29 wordp:0x5d9f size:0x0002 C-string:'WCHARS'
//         SKIP2NE  codep:0x1d29 wordp:0x5dad size:0x0002 C-string:'SKIP2NE'
//            -AIN  codep:0x1d29 wordp:0x5db8 size:0x0002 C-string:'_dash_AIN'
//         'LAUNCH  codep:0x1d29 wordp:0x5dc6 size:0x0002 C-string:'_i_LAUNCH'
//         ?ON-PLA  codep:0x1d29 wordp:0x5dd4 size:0x0002 C-string:'IsON_dash_PLA'
//         ?RECALL  codep:0x1d29 wordp:0x5de2 size:0x0002 C-string:'IsRECALL'
//            WMSG  codep:0x1d29 wordp:0x5ded size:0x0002 C-string:'WMSG'
//             CTX  codep:0x1d29 wordp:0x5df7 size:0x0002 C-string:'CTX'
//             CTY  codep:0x1d29 wordp:0x5e01 size:0x0002 C-string:'CTY'
//           FTRIG  codep:0x1d29 wordp:0x5e0d size:0x0002 C-string:'FTRIG'
//           FQUIT  codep:0x1d29 wordp:0x5e19 size:0x0002 C-string:'FQUIT'
//            LKEY  codep:0x1d29 wordp:0x5e24 size:0x0002 C-string:'LKEY'
//         'BUTTON  codep:0x1d29 wordp:0x5e32 size:0x0002 C-string:'_i_BUTTON'
//         BTN-REC  codep:0x1d29 wordp:0x5e40 size:0x0002 C-string:'BTN_dash_REC'
//         CRSCOLO  codep:0x1d29 wordp:0x5e4e size:0x0002 C-string:'CRSCOLO'
//           W5E52  codep:0x1d29 wordp:0x5e52 size:0x0002 C-string:'W5E52'
//            ?>OP  codep:0x1d29 wordp:0x5e5d size:0x0002 C-string:'Is_gt_OP'
//           'YANK  codep:0x1d29 wordp:0x5e69 size:0x0002 C-string:'_i_YANK'
//             ?12  codep:0x1d29 wordp:0x5e73 size:0x0002 C-string:'Is12'
//          '+VESS  codep:0x1d29 wordp:0x5e80 size:0x0002 C-string:'_i__plus_VESS'
//            ?NEB  codep:0x1d29 wordp:0x5e8b size:0x0002 C-string:'IsNEB'
//         FORCEKE  codep:0x1d29 wordp:0x5e99 size:0x0002 C-string:'FORCEKE'
//            %VAL  codep:0x1d29 wordp:0x5ea4 size:0x0002 C-string:'_pe_VAL'
//         SCROLL-  codep:0x1d29 wordp:0x5eb2 size:0x0002 C-string:'SCROLL_dash_'
//         [#CACHE  codep:0x1d29 wordp:0x5ec0 size:0x0002 C-string:'_bo__n_CACHE'
//          ESC-EN  codep:0x1d29 wordp:0x5ecd size:0x0002 C-string:'ESC_dash_EN'
//         ESC-PFA  codep:0x1d29 wordp:0x5edb size:0x0002 C-string:'ESC_dash_PFA'
//         LINE-CO  codep:0x1d29 wordp:0x5ee9 size:0x0002 C-string:'LINE_dash_CO'
//          PM-PTR  codep:0x1d29 wordp:0x5ef6 size:0x0002 C-string:'PM_dash_PTR'
//           W5EFA  codep:0x1d29 wordp:0x5efa size:0x0002 C-string:'W5EFA'
//            SKEY  codep:0x1d29 wordp:0x5f05 size:0x0002 C-string:'SKEY'
//            #AUX  codep:0x1d29 wordp:0x5f10 size:0x0002 C-string:'_n_AUX'
//            ?EGA  codep:0x1d29 wordp:0x5f1b size:0x0002 C-string:'IsEGA'
//            ?5:(  codep:0x1d29 wordp:0x5f26 size:0x0002 C-string:'Is5_c__ro_'
//            XABS  codep:0x1d29 wordp:0x5f31 size:0x0002 C-string:'XABS'
//            YABS  codep:0x1d29 wordp:0x5f3c size:0x0002 C-string:'YABS'
//         HEADING  codep:0x1d29 wordp:0x5f4a size:0x0002 C-string:'HEADING'
//           3DSEG  codep:0x1d29 wordp:0x5f56 size:0x0002 C-string:'_3DSEG'
//            VIN'  codep:0x1d29 wordp:0x5f61 size:0x0002 C-string:'VIN_i_'
//         YSCREEN  codep:0x1d29 wordp:0x5f6f size:0x0000 C-string:'YSCREEN'
//           W5F71  codep:0x7420 wordp:0x5f71 size:0x0000 C-string:'W5F71'
//         XSCREEN  codep:0x1d29 wordp:0x5f7d size:0x0002 C-string:'XSCREEN'
//         'COMBAT  codep:0x1d29 wordp:0x5f8b size:0x0002 C-string:'_i_COMBAT'
//           'CEX+  codep:0x1d29 wordp:0x5f97 size:0x0002 C-string:'_i_CEX_plus_'
//            'CEX  codep:0x1d29 wordp:0x5fa2 size:0x0002 C-string:'_i_CEX'
//            'WAX  codep:0x1d29 wordp:0x5fad size:0x0002 C-string:'_i_WAX'
//         TERMINA  codep:0x1d29 wordp:0x5fbb size:0x0002 C-string:'TERMINA'
//         ?COMBAT  codep:0x1d29 wordp:0x5fc9 size:0x0002 C-string:'IsCOMBAT'
//         ?ATTACK  codep:0x1d29 wordp:0x5fd7 size:0x0002 C-string:'IsATTACK'
//            TBOX  codep:0x1d29 wordp:0x5fe2 size:0x0002 C-string:'TBOX'
//           W5FE6  codep:0x1d29 wordp:0x5fe6 size:0x0002 C-string:'W5FE6'
//         STAR-HR  codep:0x1d29 wordp:0x5ff4 size:0x0002 C-string:'STAR_dash_HR'
//         STARDAT  codep:0x1d29 wordp:0x6002 size:0x0002 C-string:'STARDAT'
//         TIME-PA  codep:0x1d29 wordp:0x6010 size:0x0002 C-string:'TIME_dash_PA'
//         #CLRMAP  codep:0x1d29 wordp:0x601e size:0x0002 C-string:'_n_CLRMAP'
//            PLHI  codep:0x1d29 wordp:0x6029 size:0x0002 C-string:'PLHI'
//         'PROCES  codep:0x1d29 wordp:0x6037 size:0x0002 C-string:'_i_PROCES'
//          CURSEG  codep:0x1d29 wordp:0x6044 size:0x0002 C-string:'CURSEG'
//         'SIMULA  codep:0x1d29 wordp:0x6052 size:0x0002 C-string:'_i_SIMULA'
//         'ENDING  codep:0x1d29 wordp:0x6060 size:0x0002 C-string:'_i_ENDING'
//         [KEYINT  codep:0x1d29 wordp:0x606e size:0x0002 C-string:'_bo_KEYINT'
//            SIL^  codep:0x1d29 wordp:0x6079 size:0x0002 C-string:'SIL_h_'
//            PIC^  codep:0x1d29 wordp:0x6084 size:0x0002 C-string:'PIC_h_'
//         'CLEANU  codep:0x1d29 wordp:0x6092 size:0x0002 C-string:'_i_CLEANU'
//         'KEY-CA  codep:0x1d29 wordp:0x60a0 size:0x0002 C-string:'_i_KEY_dash_CA'
//         '.VITAL  codep:0x1d29 wordp:0x60ae size:0x0002 C-string:'_i__dot_VITAL'
//          '.DATE  codep:0x1d29 wordp:0x60bb size:0x0002 C-string:'_i__dot_DATE'
//         '.VEHIC  codep:0x1d29 wordp:0x60c9 size:0x0002 C-string:'_i__dot_VEHIC'
//         'VEHICL  codep:0x1d29 wordp:0x60d7 size:0x0002 C-string:'_i_VEHICL'
//         'CREW-C  codep:0x1d29 wordp:0x60e5 size:0x0002 C-string:'_i_CREW_dash_C'
//         'EXTERN  codep:0x1d29 wordp:0x60f3 size:0x0002 C-string:'_i_EXTERN'
//         'REPAIR  codep:0x1d29 wordp:0x6101 size:0x0002 C-string:'_i_REPAIR'
//         'TREATM  codep:0x1d29 wordp:0x610f size:0x0002 C-string:'_i_TREATM'
//         WEAPON-  codep:0x1d29 wordp:0x611d size:0x0002 C-string:'WEAPON_dash_'
//           ^CRIT  codep:0x1d29 wordp:0x6129 size:0x0002 C-string:'_h_CRIT'
//           ?FLAT  codep:0x1d29 wordp:0x6135 size:0x0002 C-string:'IsFLAT'
//             1ST  codep:0x1d29 wordp:0x613f size:0x0002 C-string:'_1ST'
//            ?MVT  codep:0x1d29 wordp:0x614a size:0x0002 C-string:'IsMVT'
//            ?SUP  codep:0x1d29 wordp:0x6155 size:0x0002 C-string:'IsSUP'
//           E-USE  codep:0x1d29 wordp:0x6161 size:0x0002 C-string:'E_dash_USE'
//         'ENERGY  codep:0x1d29 wordp:0x616f size:0x0002 C-string:'_i_ENERGY'
//         ?SECURE  codep:0x1d29 wordp:0x617d size:0x0002 C-string:'IsSECURE'
//            'STP  codep:0x1d29 wordp:0x6188 size:0x0002 C-string:'_i_STP'
//          'RSIDE  codep:0x1d29 wordp:0x6195 size:0x0002 C-string:'_i_RSIDE'
//          DERROR  codep:0x1d29 wordp:0x61a2 size:0x0002 C-string:'DERROR'
//             NLR  codep:0x1d29 wordp:0x61ac size:0x0002 C-string:'NLR'
//             OVT  codep:0x53f7 wordp:0x61b6 size:0x0016 C-string:'OVT'
//             ?10  codep:0x1d29 wordp:0x61d4 size:0x0002 C-string:'Is10'
//            BITS  codep:0x1d29 wordp:0x61df size:0x0002 C-string:'BITS'
//           #BITS  codep:0x1d29 wordp:0x61eb size:0x0002 C-string:'_n_BITS'
//              ?5  codep:0x1d29 wordp:0x61f4 size:0x0002 C-string:'Is5'
//         SELLING  codep:0x1d29 wordp:0x6202 size:0x0002 C-string:'SELLING'
//         BARTERI  codep:0x1d29 wordp:0x6210 size:0x0002 C-string:'BARTERI'
//          ?REPLY  codep:0x1d29 wordp:0x621d size:0x0002 C-string:'IsREPLY'
//           PLAST  codep:0x1d29 wordp:0x6229 size:0x0002 C-string:'PLAST'
//            PAST  codep:0x1d29 wordp:0x6234 size:0x0002 C-string:'PAST'
//            HAZE  codep:0x1d29 wordp:0x623f size:0x0004 C-string:'HAZE'
//         ?CALLIN  codep:0x1d29 wordp:0x624f size:0x0002 C-string:'IsCALLIN'
//          STAGES  codep:0x1d29 wordp:0x625c size:0x000e C-string:'STAGES'
//           %SLUG  codep:0x1d29 wordp:0x6274 size:0x0002 C-string:'_pe_SLUG'
//            ITEM  codep:0x1d29 wordp:0x627f size:0x0004 C-string:'ITEM'
//           FSTUN  codep:0x1d29 wordp:0x628d size:0x0002 C-string:'FSTUN'
//         PATIENC  codep:0x1d29 wordp:0x629b size:0x0002 C-string:'PATIENC'
//            TMAP  codep:0x1d29 wordp:0x62a6 size:0x0004 C-string:'TMAP'
//              :(  codep:0x1d29 wordp:0x62b1 size:0x0002 C-string:'_c__ro_'
//         TIRED-T  codep:0x1d29 wordp:0x62bf size:0x0004 C-string:'TIRED_dash_T'
//         LASTREP  codep:0x1d29 wordp:0x62cf size:0x0004 C-string:'LASTREP'
//         TALKCOU  codep:0x1d29 wordp:0x62df size:0x0004 C-string:'TALKCOU'
//          VSTIME  codep:0x1d29 wordp:0x62ee size:0x0004 C-string:'VSTIME'
//         10*CARG  codep:0x1d29 wordp:0x62fe size:0x0004 C-string:'_10_star_CARG'
//         SENSE-A  codep:0x1d29 wordp:0x630e size:0x0004 C-string:'SENSE_dash_A'
//           EYEXY  codep:0x1d29 wordp:0x631c size:0x0004 C-string:'EYEXY'
//          WEAPXY  codep:0x1d29 wordp:0x632b size:0x0004 C-string:'WEAPXY'
//          10*END  codep:0x1d29 wordp:0x633a size:0x0004 C-string:'_10_star_END'
//         TOWFINE  codep:0x1d29 wordp:0x634a size:0x0004 C-string:'TOWFINE'
//         ENC-TIM  codep:0x1d29 wordp:0x635a size:0x0004 C-string:'ENC_dash_TIM'
//         NAV-TIM  codep:0x1d29 wordp:0x636a size:0x0004 C-string:'NAV_dash_TIM'
//           W6370  codep:0x1d29 wordp:0x6370 size:0x0004 C-string:'W6370'
//           STIME  codep:0x1d29 wordp:0x637e size:0x0004 C-string:'STIME'
//           ETIME  codep:0x1d29 wordp:0x638c size:0x0002 C-string:'ETIME'
//           W6390  codep:0x1d29 wordp:0x6390 size:0x0004 C-string:'W6390'
//           W6396  codep:0x1d29 wordp:0x6396 size:0x0004 C-string:'W6396'
//           W639C  codep:0x1d29 wordp:0x639c size:0x0004 C-string:'W639C'
//           W63A2  codep:0x1d29 wordp:0x63a2 size:0x0004 C-string:'W63A2'
//         KEYTIME  codep:0x1d29 wordp:0x63b2 size:0x0004 C-string:'KEYTIME'
//         LKEYTIM  codep:0x1d29 wordp:0x63c2 size:0x0004 C-string:'LKEYTIM'
//       (SCROLL_1  codep:0x1d29 wordp:0x63d2 size:0x0004 C-string:'_ro_SCROLL_1'
//         (ORIGIN  codep:0x1d29 wordp:0x63e2 size:0x0004 C-string:'_ro_ORIGIN'
//       (SCROLL_2  codep:0x1d29 wordp:0x63f2 size:0x0004 C-string:'_ro_SCROLL_2'
//         REAL-MS  codep:0x1d29 wordp:0x6402 size:0x0004 C-string:'REAL_dash_MS'
//         LAST-UP  codep:0x1d29 wordp:0x6412 size:0x0004 C-string:'LAST_dash_UP'
//         XWLD:XP  codep:0x1d29 wordp:0x6422 size:0x0004 C-string:'XWLD_c_XP'
//         YWLD:YP  codep:0x1d29 wordp:0x6432 size:0x0004 C-string:'YWLD_c_YP'
//          ANCHOR  codep:0x1d29 wordp:0x6441 size:0x0004 C-string:'ANCHOR'
//         OK-TALK  codep:0x1d29 wordp:0x6451 size:0x0004 C-string:'OK_dash_TALK'
//         TVEHICL  codep:0x1d29 wordp:0x6461 size:0x0004 C-string:'TVEHICL'
//         TV-HOLD  codep:0x1d29 wordp:0x6471 size:0x0004 C-string:'TV_dash_HOLD'
//         SUPER-B  codep:0x1d29 wordp:0x6481 size:0x0004 C-string:'SUPER_dash_B'
//         (SYSTEM  codep:0x1d29 wordp:0x6491 size:0x0004 C-string:'_ro_SYSTEM'
//         (ORBIT)  codep:0x1d29 wordp:0x64a1 size:0x0004 C-string:'_ro_ORBIT_rc_'
//         (PLANET  codep:0x1d29 wordp:0x64b1 size:0x0004 C-string:'_ro_PLANET'
//         (SURFAC  codep:0x1d29 wordp:0x64c1 size:0x0004 C-string:'_ro_SURFAC'
//         (ENCOUN  codep:0x1d29 wordp:0x64d1 size:0x0004 C-string:'_ro_ENCOUN'
//         (SHIPBO  codep:0x1d29 wordp:0x64e1 size:0x0004 C-string:'_ro_SHIPBO'
//         (AORIGI  codep:0x1d29 wordp:0x64f1 size:0x0004 C-string:'_ro_AORIGI'
//         THIS-RE  codep:0x1d29 wordp:0x6501 size:0x0004 C-string:'THIS_dash_RE'
//         (THIS-I  codep:0x1d29 wordp:0x6511 size:0x0004 C-string:'_ro_THIS_dash_I'
//           W6517  codep:0x1d29 wordp:0x6517 size:0x0002 C-string:'W6517'
//           W651B  codep:0x1d29 wordp:0x651b size:0x00ba C-string:'W651B'
//            IBFR  codep:0x1d29 wordp:0x65de size:0x0111 C-string:'IBFR'
//           LSCAN  codep:0x1d29 wordp:0x66f9 size:0x0190 C-string:'LSCAN'
//           W688B  codep:0x1d29 wordp:0x688b size:0x0040 C-string:'W688B'
//           W68CD  codep:0x1d29 wordp:0x68cd size:0x0040 C-string:'W68CD'
//           W690F  codep:0x1d29 wordp:0x690f size:0x0040 C-string:'W690F'
//           W6951  codep:0x1d29 wordp:0x6951 size:0x0040 C-string:'W6951'
//            CMAP  codep:0x1d29 wordp:0x699a size:0x0040 C-string:'CMAP'
//           ATIME  codep:0x1d29 wordp:0x69e4 size:0x0004 C-string:'ATIME'
//           LRTRN  codep:0x1d29 wordp:0x69f2 size:0x0004 C-string:'LRTRN'
//         (TRADER  codep:0x1d29 wordp:0x6a02 size:0x0004 C-string:'_ro_TRADER'
//         (THING)  codep:0x1d29 wordp:0x6a12 size:0x0004 C-string:'_ro_THING_rc_'
//         BUY-LIS  codep:0x3b74 wordp:0x6a22 size:0x0004 C-string:'BUY_dash_LIS'
//          #ETIME  codep:0x1d29 wordp:0x6a31 size:0x0004 C-string:'_n_ETIME'
//           NTIME  codep:0x1d29 wordp:0x6a3f size:0x0004 C-string:'NTIME'
//            'UHL  codep:0x1d29 wordp:0x6a4c size:0x0002 C-string:'_i_UHL'
//         MERCATO  codep:0x4e6f wordp:0x6a5a size:0x0008 C-string:'MERCATO'
//         CONANCH  codep:0x4e6f wordp:0x6a6e size:0x0008 C-string:'CONANCH'
//         CONTOUR  codep:0x4e6f wordp:0x6a82 size:0x0008 C-string:'CONTOUR'
//         ICONIMA  codep:0x4e6f wordp:0x6a96 size:0x0008 C-string:'ICONIMA'
//          VERTEX  codep:0x4e6f wordp:0x6aa9 size:0x0008 C-string:'VERTEX'
//           FACET  codep:0x4e6f wordp:0x6abb size:0x0008 C-string:'FACET'
//            FACE  codep:0x4e6f wordp:0x6acc size:0x0008 C-string:'FACE'
//           PPOLY  codep:0x4e6f wordp:0x6ade size:0x0008 C-string:'PPOLY'
//         GVERTEX  codep:0x4e6f wordp:0x6af2 size:0x0008 C-string:'GVERTEX'
//           GPOLY  codep:0x4e6f wordp:0x6b04 size:0x0008 C-string:'GPOLY'
//         GRIDCOL  codep:0x4e6f wordp:0x6b18 size:0x0008 C-string:'GRIDCOL'
//          G1VERT  codep:0x4e6f wordp:0x6b2b size:0x0008 C-string:'G1VERT'
//         9X9COAR  codep:0x4e6f wordp:0x6b3f size:0x0008 C-string:'_9X9COAR'
//         9X9FINE  codep:0x4e6f wordp:0x6b53 size:0x0008 C-string:'_9X9FINE'
//         XFORMVE  codep:0x4e6f wordp:0x6b67 size:0x0008 C-string:'XFORMVE'
//         IARRAYS  codep:0x53f7 wordp:0x6b7b size:0x001e C-string:'IARRAYS'
//         *STARPO  codep:0x3b74 wordp:0x6ba5 size:0x0004 C-string:'_star_STARPO'
//         *PERSON  codep:0x3b74 wordp:0x6bb5 size:0x0004 C-string:'_star_PERSON'
//         *SHIP-C  codep:0x3b74 wordp:0x6bc5 size:0x0004 C-string:'_star_SHIP_dash_C'
//         *TRADE-  codep:0x3b74 wordp:0x6bd5 size:0x0004 C-string:'_star_TRADE_dash_'
//         *ITEMS-  codep:0x3b74 wordp:0x6be5 size:0x0004 C-string:'_star_ITEMS_dash_'
//         *OPERAT  codep:0x3b74 wordp:0x6bf5 size:0x0004 C-string:'_star_OPERAT'
//           *BANK  codep:0x3b74 wordp:0x6c03 size:0x0004 C-string:'_star_BANK'
//         *STARSH  codep:0x3b74 wordp:0x6c13 size:0x0004 C-string:'_star_STARSH'
//            *EYE  codep:0x3b74 wordp:0x6c20 size:0x0004 C-string:'_star_EYE'
//          *PLIST  codep:0x3b74 wordp:0x6c2f size:0x0004 C-string:'_star_PLIST'
//           *MISS  codep:0x3b74 wordp:0x6c3d size:0x0004 C-string:'_star_MISS'
//           *MESS  codep:0x3b74 wordp:0x6c4b size:0x0004 C-string:'_star_MESS'
//         'VERSIO  codep:0x1d29 wordp:0x6c5b size:0x0002 C-string:'_i_VERSIO'
//           CTTOP  codep:0x1d29 wordp:0x6c67 size:0x0002 C-string:'CTTOP'
//           CTBOT  codep:0x1d29 wordp:0x6c73 size:0x0002 C-string:'CTBOT'
//        >0FONT_1  codep:0x224c wordp:0x6c80 size:0x0026 C-string:'_gt_0FONT_1'
//         ?UPDATE  codep:0x6cb2 wordp:0x6cb2 size:0x0053 C-string:'IsUPDATE'
//            C!_2  codep:0x224c wordp:0x6d0c size:0x0006 C-string:'C_ex__2'
//             !_2  codep:0x224c wordp:0x6d18 size:0x0006 C-string:'Store_2'
//            +!_2  codep:0x224c wordp:0x6d25 size:0x0006 C-string:'_plus__ex__2'
//          1.5!_2  codep:0x224c wordp:0x6d34 size:0x0006 C-string:'_1_dot_5_ex__2'
//            2!_2  codep:0x224c wordp:0x6d41 size:0x0006 C-string:'_2_ex__2'
//              D!  codep:0x224c wordp:0x6d4e size:0x0004 C-string:'StoreD'
//            ON_2  codep:0x224c wordp:0x6d59 size:0x0006 C-string:'ON_2'
//             099  codep:0x224c wordp:0x6d67 size:0x0006 C-string:'_099'
//            2OFF  codep:0x224c wordp:0x6d76 size:0x000c C-string:'_2OFF'
//         CMOVE_2  codep:0x224c wordp:0x6d8c size:0x000a C-string:'CMOVE_2'
//          FILL_2  codep:0x224c wordp:0x6d9f size:0x000c C-string:'FILL_2'
//           W6DAD  codep:0x224c wordp:0x6dad size:0x000a C-string:'W6DAD'
//         BLOCK_2  codep:0x224c wordp:0x6dc1 size:0x0006 C-string:'BLOCK_2'
//        LBLOCK_2  codep:0x224c wordp:0x6dd2 size:0x0006 C-string:'LBLOCK_2'
//          LOAD_2  codep:0x224c wordp:0x6de1 size:0x000c C-string:'LOAD_2'
//           -TEXT  codep:0x224c wordp:0x6df7 size:0x0036 C-string:'_dash_TEXT'
//              $=  codep:0x224c wordp:0x6e34 size:0x0022 C-string:'_do__eq_'
//          SIGFLD  codep:0x224c wordp:0x6e61 size:0x0013 C-string:'SIGFLD'
//         :SIGNAT  codep:0x6e67 wordp:0x6e80 size:0x0002 C-string:'_c_SIGNAT'
//         :TIMEST  codep:0x6e67 wordp:0x6e8e size:0x0002 C-string:'_c_TIMEST'
//          :CKSUM  codep:0x6e67 wordp:0x6e9b size:0x0002 C-string:'_c_CKSUM'
//           :SAVE  codep:0x6e67 wordp:0x6ea7 size:0x0002 C-string:'_c_SAVE'
//         :VERSIO  codep:0x6e67 wordp:0x6eb5 size:0x0002 C-string:'_c_VERSIO'
//          THRU_2  codep:0x224c wordp:0x6ec0 size:0x0024 C-string:'THRU_2'
//          VA>BLK  codep:0x6eef wordp:0x6eef size:0x0019 C-string:'VA_gt_BLK'
//          VA>BUF  codep:0x224c wordp:0x6f13 size:0x0008 C-string:'VA_gt_BUF'
//           W6F1D  codep:0x224c wordp:0x6f1d size:0x0008 C-string:'W6F1D'
//           W6F27  codep:0x224c wordp:0x6f27 size:0x000c C-string:'W6F27'
//           W6F35  codep:0x224c wordp:0x6f35 size:0x0012 C-string:'W6F35'
//           W6F49  codep:0x224c wordp:0x6f49 size:0x000c C-string:'W6F49'
//           W6F57  codep:0x224c wordp:0x6f57 size:0x0006 C-string:'W6F57'
//         MOUNTFI  codep:0x224c wordp:0x6f69 size:0x001a C-string:'MOUNTFI'
//           W6F85  codep:0x224c wordp:0x6f85 size:0x000a C-string:'W6F85'
//           W6F91  codep:0x224c wordp:0x6f91 size:0x000a C-string:'W6F91'
//          MOUNTA  codep:0x224c wordp:0x6fa6 size:0x0019 C-string:'MOUNTA'
//          MOUNTB  codep:0x224c wordp:0x6fca size:0x0019 C-string:'MOUNTB'
//           W6FE5  codep:0x224c wordp:0x6fe5 size:0x0016 C-string:'W6FE5'
//           W6FFD  codep:0x224c wordp:0x6ffd size:0x0010 C-string:'W6FFD'
//           W700F  codep:0x224c wordp:0x700f size:0x0010 C-string:'W700F'
//           W7021  codep:0x224c wordp:0x7021 size:0x0052 C-string:'W7021'
//           W7075  codep:0x224c wordp:0x7075 size:0x000c C-string:'W7075'
//           W7083  codep:0x224c wordp:0x7083 size:0x008e C-string:'W7083'
//         MEM>DSK  codep:0x224c wordp:0x711d size:0x0006 C-string:'MEM_gt_DSK'
//         MEM<DSK  codep:0x224c wordp:0x712f size:0x0006 C-string:'MEM_st_DSK'
//           W7137  codep:0x7137 wordp:0x7137 size:0x004e C-string:'W7137'
//           W7187  codep:0x7187 wordp:0x7187 size:0x0042 C-string:'W7187'
//           W71CB  codep:0x224c wordp:0x71cb size:0x0017 C-string:'W71CB'
//         FILE-NA  codep:0x71d1 wordp:0x71ee size:0x0001 C-string:'FILE_dash_NA'
//         FILE-TY  codep:0x71d1 wordp:0x71fb size:0x0001 C-string:'FILE_dash_TY'
//         FILE-ST  codep:0x71d1 wordp:0x7208 size:0x0001 C-string:'FILE_dash_ST'
//         FILE-EN  codep:0x71d1 wordp:0x7215 size:0x0001 C-string:'FILE_dash_EN'
//         FILE-#R  codep:0x71d1 wordp:0x7222 size:0x0001 C-string:'FILE_dash__n_R'
//         FILE-RL  codep:0x71d1 wordp:0x722f size:0x0001 C-string:'FILE_dash_RL'
//         FILE-SL  codep:0x71d1 wordp:0x723c size:0x0001 C-string:'FILE_dash_SL'
//           W723F  codep:0x723f wordp:0x723f size:0x001a C-string:'W723F'
//           FILE:  codep:0x224c wordp:0x7263 size:0x005c C-string:'FILE_c_'
//           >FILE  codep:0x224c wordp:0x72c9 size:0x0010 C-string:'_gt_FILE'
//           FILE<  codep:0x224c wordp:0x72e3 size:0x0010 C-string:'FILE_st_'
//           W72F5  codep:0x224c wordp:0x72f5 size:0x000a C-string:'W72F5'
//           W7301  codep:0x224c wordp:0x7301 size:0x0020 C-string:'W7301'
//           W7323  codep:0x224c wordp:0x7323 size:0x0016 C-string:'W7323'
//         @RECORD  codep:0x224c wordp:0x7345 size:0x0022 C-string:'GetRECORD'
//          AFIELD  codep:0x224c wordp:0x7372 size:0x0084 C-string:'AFIELD'
//           W73F8  codep:0x73f8 wordp:0x73f8 size:0x000f C-string:'W73F8'
//          IFIELD  codep:0x224c wordp:0x7412 size:0x0046 C-string:'IFIELD'
//         INST-SI  codep:0x7420 wordp:0x7464 size:0x0003 C-string:'INST_dash_SI'
//         INST-PR  codep:0x7420 wordp:0x7473 size:0x0003 C-string:'INST_dash_PR'
//         INST-OF  codep:0x7420 wordp:0x7482 size:0x0003 C-string:'INST_dash_OF'
//         INST-CL  codep:0x7420 wordp:0x7491 size:0x0003 C-string:'INST_dash_CL'
//         INST-SP  codep:0x7420 wordp:0x74a0 size:0x0003 C-string:'INST_dash_SP'
//         INST-QT  codep:0x7420 wordp:0x74af size:0x0003 C-string:'INST_dash_QT'
//          INST-X  codep:0x7420 wordp:0x74bd size:0x0003 C-string:'INST_dash_X'
//          INST-Y  codep:0x7420 wordp:0x74cb size:0x0003 C-string:'INST_dash_Y'
//           W74D0  codep:0x224c wordp:0x74d0 size:0x0006 C-string:'W74D0'
//           W74D8  codep:0x224c wordp:0x74d8 size:0x0006 C-string:'W74D8'
//           W74E0  codep:0x224c wordp:0x74e0 size:0x0006 C-string:'W74E0'
//           W74E8  codep:0x224c wordp:0x74e8 size:0x0006 C-string:'W74E8'
//         !INST-S  codep:0x224c wordp:0x74fa size:0x0006 C-string:'StoreINST_dash_S'
//           W7502  codep:0x224c wordp:0x7502 size:0x0006 C-string:'W7502'
//           W750A  codep:0x224c wordp:0x750a size:0x0006 C-string:'W750A'
//           W7512  codep:0x224c wordp:0x7512 size:0x0006 C-string:'W7512'
//         @INST-C  codep:0x224c wordp:0x7524 size:0x0006 C-string:'GetINST_dash_C'
//         @INST-S  codep:0x224c wordp:0x7536 size:0x0006 C-string:'GetINST_dash_S'
//         =SPECIE  codep:0x224c wordp:0x7548 size:0x0006 C-string:'_eq_SPECIE'
//              >C  codep:0x7555 wordp:0x7555 size:0x0014 C-string:'_gt_C'
//              C>  codep:0x7570 wordp:0x7570 size:0x0016 C-string:'C_gt_'
//              CI  codep:0x758d wordp:0x758d size:0x0012 C-string:'CI'
//           CDROP  codep:0x224c wordp:0x75a9 size:0x0006 C-string:'CDROP'
//             CI'  codep:0x224c wordp:0x75b7 size:0x000a C-string:'CI_i_'
//              CJ  codep:0x224c wordp:0x75c8 size:0x000a C-string:'CJ'
//           COVER  codep:0x224c wordp:0x75dc size:0x0006 C-string:'COVER'
//          CDEPTH  codep:0x75ed wordp:0x75ed size:0x0012 C-string:'CDEPTH'
//           ?NULL  codep:0x224c wordp:0x7609 size:0x0006 C-string:'IsNULL'
//          ?-NULL  codep:0x224c wordp:0x761a size:0x0006 C-string:'Is_dash_NULL'
//          ?CHILD  codep:0x224c wordp:0x762b size:0x0008 C-string:'IsCHILD'
//           W7635  codep:0x224c wordp:0x7635 size:0x0008 C-string:'W7635'
//          !IADDR  codep:0x224c wordp:0x7648 size:0x0006 C-string:'StoreIADDR'
//           W7650  codep:0x7650 wordp:0x7650 size:0x000a C-string:'W7650'
//           W765C  codep:0x224c wordp:0x765c size:0x0014 C-string:'W765C'
//           W7672  codep:0x224c wordp:0x7672 size:0x0014 C-string:'W7672'
//           W7688  codep:0x224c wordp:0x7688 size:0x0006 C-string:'W7688'
//         PRIORIT  codep:0x769a wordp:0x769a size:0x0032 C-string:'PRIORIT'
//           W76CE  codep:0x224c wordp:0x76ce size:0x0032 C-string:'W76CE'
//           W7702  codep:0x224c wordp:0x7702 size:0x0032 C-string:'W7702'
//           W7736  codep:0x224c wordp:0x7736 size:0x0066 C-string:'W7736'
//           W779E  codep:0x224c wordp:0x779e size:0x0006 C-string:'W779E'
//           W77A6  codep:0x224c wordp:0x77a6 size:0x0012 C-string:'W77A6'
//           W77BA  codep:0x224c wordp:0x77ba size:0x001a C-string:'W77BA'
//           W77D6  codep:0x224c wordp:0x77d6 size:0x003e C-string:'W77D6'
//           W7816  codep:0x224c wordp:0x7816 size:0x007a C-string:'W7816'
//           W7892  codep:0x224c wordp:0x7892 size:0x001c C-string:'W7892'
//            1BTN  codep:0x7394 wordp:0x78b7 size:0x0006 C-string:'_1BTN'
//         AFIELD:  codep:0x224c wordp:0x78c9 size:0x0028 C-string:'AFIELD_c_'
//         IFIELD:  codep:0x224c wordp:0x78fd size:0x0014 C-string:'IFIELD_c_'
//            #BTN  codep:0x7394 wordp:0x791a size:0x0006 C-string:'_n_BTN'
//           W7922  codep:0x224c wordp:0x7922 size:0x0074 C-string:'W7922'
//         SET-CUR  codep:0x224c wordp:0x79a2 size:0x0034 C-string:'SET_dash_CUR'
//          ICLOSE  codep:0x224c wordp:0x79e1 size:0x0020 C-string:'ICLOSE'
//          *CLOSE  codep:0x224c wordp:0x7a0c size:0x000c C-string:'_star_CLOSE'
//            >C+S  codep:0x224c wordp:0x7a21 size:0x0006 C-string:'_gt_C_plus_S'
//           @>C+S  codep:0x224c wordp:0x7a31 size:0x0006 C-string:'Get_gt_C_plus_S'
//           IOPEN  codep:0x224c wordp:0x7a41 size:0x000c C-string:'IOPEN'
//            CCLR  codep:0x224c wordp:0x7a56 size:0x0014 C-string:'CCLR'
//           W7A6C  codep:0x224c wordp:0x7a6c size:0x0006 C-string:'W7A6C'
//           ?LAST  codep:0x224c wordp:0x7a7c size:0x0008 C-string:'IsLAST'
//          ?FIRST  codep:0x224c wordp:0x7a8f size:0x0008 C-string:'IsFIRST'
//             >C+  codep:0x224c wordp:0x7a9f size:0x000a C-string:'_gt_C_plus_'
//           INEXT  codep:0x224c wordp:0x7ab3 size:0x000e C-string:'INEXT'
//           IPREV  codep:0x224c wordp:0x7acb size:0x000e C-string:'IPREV'
//          IFIRST  codep:0x224c wordp:0x7ae4 size:0x0008 C-string:'IFIRST'
//           ILAST  codep:0x224c wordp:0x7af6 size:0x0006 C-string:'ILAST'
//            VCLR  codep:0x224c wordp:0x7b05 size:0x0008 C-string:'VCLR'
//              >V  codep:0x7b14 wordp:0x7b14 size:0x0010 C-string:'_gt_V'
//              V>  codep:0x7b2b wordp:0x7b2b size:0x0010 C-string:'V_gt_'
//              VI  codep:0x7b42 wordp:0x7b42 size:0x000c C-string:'VI'
//           W7B50  codep:0x224c wordp:0x7b50 size:0x0020 C-string:'W7B50'
//         SAVE-BU  codep:0x224c wordp:0x7b7c size:0x0006 C-string:'SAVE_dash_BU'
//         FLUSH_2  codep:0x224c wordp:0x7b8c size:0x0006 C-string:'FLUSH_2'
//              MT  codep:0x224c wordp:0x7b99 size:0x0006 C-string:'MT'
//         IINSERT  codep:0x224c wordp:0x7bab size:0x003c C-string:'IINSERT'
//         <INSERT  codep:0x224c wordp:0x7bf3 size:0x001a C-string:'_st_INSERT'
//         >INSERT  codep:0x224c wordp:0x7c19 size:0x000a C-string:'_gt_INSERT'
//         IEXTRAC  codep:0x224c wordp:0x7c2f size:0x0040 C-string:'IEXTRAC'
//           W7C71  codep:0x224c wordp:0x7c71 size:0x0006 C-string:'W7C71'
//         @NEWSPA  codep:0x224c wordp:0x7c83 size:0x0006 C-string:'GetNEWSPA'
//         !NEWSPA  codep:0x224c wordp:0x7c95 size:0x0006 C-string:'StoreNEWSPA'
//           W7C9D  codep:0x224c wordp:0x7c9d size:0x000a C-string:'W7C9D'
//         MAXSPAC  codep:0x224c wordp:0x7cb3 size:0x000e C-string:'MAXSPAC'
//           W7CC3  codep:0x224c wordp:0x7cc3 size:0x0010 C-string:'W7CC3'
//            FLD@  codep:0x4b3b wordp:0x7cdc size:0x0010 C-string:'FLD_at_'
//            FLD!  codep:0x4b3b wordp:0x7cf5 size:0x0010 C-string:'FLD_ex_'
//           IFLD@  codep:0x224c wordp:0x7d0f size:0x0010 C-string:'IFLD_at_'
//           IFLD!  codep:0x224c wordp:0x7d29 size:0x0010 C-string:'IFLD_ex_'
//           W7D3B  codep:0x224c wordp:0x7d3b size:0x000a C-string:'W7D3B'
//         ?CLASS/  codep:0x224c wordp:0x7d51 size:0x0020 C-string:'IsCLASS_slash_'
//           W7D73  codep:0x224c wordp:0x7d73 size:0x0006 C-string:'W7D73'
//           W7D7B  codep:0x224c wordp:0x7d7b size:0x0006 C-string:'W7D7B'
//           W7D83  codep:0x224c wordp:0x7d83 size:0x0006 C-string:'W7D83'
//             MAP  codep:0x224c wordp:0x7d91 size:0x0006 C-string:'MAP'
//           W7D99  codep:0x224c wordp:0x7d99 size:0x000a C-string:'W7D99'
//           W7DA5  codep:0x224c wordp:0x7da5 size:0x0008 C-string:'W7DA5'
//          SELECT  codep:0x224c wordp:0x7db8 size:0x0028 C-string:'SELECT'
//         ?>FIRST  codep:0x224c wordp:0x7dec size:0x000c C-string:'Is_gt_FIRST'
//         SELECT-  codep:0x224c wordp:0x7e04 size:0x002a C-string:'SELECT_dash_'
//           W7E30  codep:0x224c wordp:0x7e30 size:0x000a C-string:'W7E30'
//           W7E3C  codep:0x224c wordp:0x7e3c size:0x0008 C-string:'W7E3C'
//           IFIND  codep:0x224c wordp:0x7e4e size:0x0008 C-string:'IFIND'
//           W7E58  codep:0x224c wordp:0x7e58 size:0x0018 C-string:'W7E58'
//         MAP>LEA  codep:0x224c wordp:0x7e7c size:0x0018 C-string:'MAP_gt_LEA'
//         MAKE1ST  codep:0x224c wordp:0x7ea0 size:0x0012 C-string:'MAKE1ST'
//           W7EB4  codep:0x224c wordp:0x7eb4 size:0x0032 C-string:'W7EB4'
//         >INACTI  codep:0x224c wordp:0x7ef2 size:0x001a C-string:'_gt_INACTI'
//           W7F0E  codep:0x224c wordp:0x7f0e size:0x0006 C-string:'W7F0E'
//         IDELETE  codep:0x224c wordp:0x7f20 size:0x002a C-string:'IDELETE'
//           W7F4C  codep:0x224c wordp:0x7f4c size:0x003a C-string:'W7F4C'
//             ALL  codep:0x224c wordp:0x7f8e size:0x0014 C-string:'ALL'
//            EACH  codep:0x224c wordp:0x7fab size:0x0010 C-string:'EACH'
//           W7FBD  codep:0x224c wordp:0x7fbd size:0x0006 C-string:'W7FBD'
//         NULLPOI  codep:0x224c wordp:0x7fcf size:0x000e C-string:'NULLPOI'
//           W7FDF  codep:0x224c wordp:0x7fdf size:0x0014 C-string:'W7FDF'
//           W7FF5  codep:0x224c wordp:0x7ff5 size:0x0034 C-string:'W7FF5'
//           W802B  codep:0x224c wordp:0x802b size:0x0040 C-string:'W802B'
//         ?>MAXSP  codep:0x224c wordp:0x8077 size:0x0014 C-string:'Is_gt_MAXSP'
//           W808D  codep:0x224c wordp:0x808d size:0x0038 C-string:'W808D'
//           W80C7  codep:0x224c wordp:0x80c7 size:0x0042 C-string:'W80C7'
//         SET?REU  codep:0x4b3b wordp:0x8115 size:0x0044 C-string:'SET_ask_REU'
//         VICREAT  codep:0x224c wordp:0x8165 size:0x003a C-string:'VICREAT'
//         ICREATE  codep:0x224c wordp:0x81ab size:0x0048 C-string:'ICREATE'
//         BOX-NAM  codep:0x7394 wordp:0x81ff size:0x0006 C-string:'BOX_dash_NAM'
//         *CREATE  codep:0x224c wordp:0x8211 size:0x0026 C-string:'_star_CREATE'
//              .C  codep:0x224c wordp:0x823e size:0x0039 C-string:'DrawC'
//           W8279  codep:0x224c wordp:0x8279 size:0x0006 C-string:'W8279'
//         OV-CANC  codep:0x224c wordp:0x828b size:0x003a C-string:'OV_dash_CANC'
//           W82C7  codep:0x224c wordp:0x82c7 size:0x000a C-string:'W82C7'
//           W82D3  codep:0x224c wordp:0x82d3 size:0x0019 C-string:'W82D3'
//           W82EE  codep:0x224c wordp:0x82ee size:0x0027 C-string:'W82EE'
//           W8317  codep:0x224c wordp:0x8317 size:0x006c C-string:'W8317'
//         SAVE-OV  codep:0x224c wordp:0x838f size:0x0034 C-string:'SAVE_dash_OV'
//         OVERLAY  codep:0x224c wordp:0x83cf size:0x0017 C-string:'OVERLAY'
//           W83E8  codep:0x224c wordp:0x83e8 size:0x0018 C-string:'W83E8'
//         OPEN-OV  codep:0x224c wordp:0x840c size:0x0052 C-string:'OPEN_dash_OV'
//           W8460  codep:0x224c wordp:0x8460 size:0x0058 C-string:'W8460'
//         CLOSE-O  codep:0x224c wordp:0x84c4 size:0x000c C-string:'CLOSE_dash_O'
//          MODULE  codep:0x224c wordp:0x84db size:0x0010 C-string:'MODULE'
//           GRSEG  codep:0x1d29 wordp:0x84f5 size:0x0002 C-string:'GRSEG'
//         SHL-BIT  codep:0x8503 wordp:0x8503 size:0x000c C-string:'SHL_dash_BIT'
//           W8511  codep:0x1d29 wordp:0x8511 size:0x0005 C-string:'W8511'
//            1PIX  codep:0x224c wordp:0x851f size:0x000e C-string:'_1PIX'
//            2PIX  codep:0x224c wordp:0x8536 size:0x000e C-string:'_2PIX'
//          GRCALL  codep:0x854f wordp:0x854f size:0x001f C-string:'GRCALL'
//          PAD|16  codep:0x224c wordp:0x8579 size:0x000a C-string:'PAD_v_16'
//         PAD>SEG  codep:0x224c wordp:0x858f size:0x0006 C-string:'PAD_gt_SEG'
//           W8597  codep:0x224c wordp:0x8597 size:0x000c C-string:'W8597'
//           @MODE  codep:0x224c wordp:0x85ad size:0x0034 C-string:'GetMODE'
//         ?HERCUL  codep:0x224c wordp:0x85ed size:0x000a C-string:'IsHERCUL'
//            ?VGA  codep:0x224c wordp:0x8600 size:0x0008 C-string:'IsVGA'
//            ?CGA  codep:0x224c wordp:0x8611 size:0x0008 C-string:'IsCGA'
//            CCGA  codep:0x1d29 wordp:0x8622 size:0x0010 C-string:'CCGA'
//            PCGA  codep:0x1d29 wordp:0x863b size:0x0010 C-string:'PCGA'
//           NBCLR  codep:0x224c wordp:0x8655 size:0x0010 C-string:'NBCLR'
//            ?XOR  codep:0x866e wordp:0x866e size:0x0026 C-string:'IsXOR'
//          !COLOR  codep:0x224c wordp:0x869f size:0x0048 C-string:'StoreCOLOR'
//          @COLOR  codep:0x224c wordp:0x86f2 size:0x0006 C-string:'GetCOLOR'
//           W86FA  codep:0x224c wordp:0x86fa size:0x0008 C-string:'W86FA'
//          RNDCLR  codep:0x224c wordp:0x870d size:0x0012 C-string:'RNDCLR'
//           W8721  codep:0x224c wordp:0x8721 size:0x0010 C-string:'W8721'
//           W8733  codep:0x224c wordp:0x8733 size:0x0012 C-string:'W8733'
//           W8747  codep:0x224c wordp:0x8747 size:0x0012 C-string:'W8747'
//           W875B  codep:0x224c wordp:0x875b size:0x0012 C-string:'W875B'
//         MON>MEM  codep:0x4b3b wordp:0x8779 size:0x0010 C-string:'MON_gt_MEM'
//         SETDBUF  codep:0x224c wordp:0x8795 size:0x0020 C-string:'SETDBUF'
//           W87B7  codep:0x1d29 wordp:0x87b7 size:0x0004 C-string:'W87B7'
//           W87BD  codep:0x224c wordp:0x87bd size:0x000c C-string:'W87BD'
//           W87CB  codep:0x224c wordp:0x87cb size:0x0026 C-string:'W87CB'
//         L@PIXEL  codep:0x87fd wordp:0x87fd size:0x0011 C-string:'L_at_PIXEL'
//           LPLOT  codep:0x8818 wordp:0x8818 size:0x0027 C-string:'LPLOT'
//          LXPLOT  codep:0x884a wordp:0x884a size:0x0027 C-string:'LXPLOT'
//           W8873  codep:0x224c wordp:0x8873 size:0x0020 C-string:'W8873'
//           BFILL  codep:0x224c wordp:0x889d size:0x0014 C-string:'BFILL'
//            DARK  codep:0x224c wordp:0x88ba size:0x0018 C-string:'DARK'
//           W88D4  codep:0x224c wordp:0x88d4 size:0x001e C-string:'W88D4'
//           LLINE  codep:0x224c wordp:0x88fc size:0x000e C-string:'LLINE'
//           FCIRC  codep:0x8914 wordp:0x8914 size:0x0015 C-string:'FCIRC'
//           W892B  codep:0x224c wordp:0x892b size:0x0010 C-string:'W892B'
//           W893D  codep:0x224c wordp:0x893d size:0x0014 C-string:'W893D'
//           W8953  codep:0x8953 wordp:0x8953 size:0x0015 C-string:'W8953'
//       .CIRCLE_1  codep:0x224c wordp:0x8974 size:0x0006 C-string:'DrawCIRCLE_1'
//         FILLCIR  codep:0x224c wordp:0x8986 size:0x0006 C-string:'FILLCIR'
//         .ELLIPS  codep:0x224c wordp:0x8998 size:0x0006 C-string:'DrawELLIPS'
//         FILLELL  codep:0x224c wordp:0x89aa size:0x0006 C-string:'FILLELL'
//           W89B2  codep:0x1d29 wordp:0x89b2 size:0x0033 C-string:'W89B2'
//           HLINE  codep:0x89ef wordp:0x89ef size:0x0008 C-string:'HLINE'
//           W89F9  codep:0x1d29 wordp:0x89f9 size:0x0004 C-string:'W89F9'
//           W89FF  codep:0x224c wordp:0x89ff size:0x002e C-string:'W89FF'
//           W8A2F  codep:0x224c wordp:0x8a2f size:0x0018 C-string:'W8A2F'
//           W8A49  codep:0x224c wordp:0x8a49 size:0x002c C-string:'W8A49'
//          >1FONT  codep:0x224c wordp:0x8a80 size:0x0006 C-string:'_gt_1FONT'
//          >2FONT  codep:0x224c wordp:0x8a91 size:0x0006 C-string:'_gt_2FONT'
//          >3FONT  codep:0x224c wordp:0x8aa2 size:0x0006 C-string:'_gt_3FONT'
//        >0FONT_2  codep:0x224c wordp:0x8ab3 size:0x0008 C-string:'_gt_0FONT_2'
//           LDPAL  codep:0x224c wordp:0x8ac5 size:0x001c C-string:'LDPAL'
//           W8AE3  codep:0x224c wordp:0x8ae3 size:0x000c C-string:'W8AE3'
//             :8>  codep:0x224c wordp:0x8af7 size:0x0006 C-string:'_c_8_gt_'
//             )8>  codep:0x224c wordp:0x8b05 size:0x0006 C-string:'_rc_8_gt_'
//            2<8>  codep:0x224c wordp:0x8b14 size:0x0006 C-string:'_2_st_8_gt_'
//             <8>  codep:0x224c wordp:0x8b22 size:0x0006 C-string:'_st_8_gt_'
//            HERC  codep:0x224c wordp:0x8b31 size:0x0010 C-string:'HERC'
//         #VECTOR  codep:0x2214 wordp:0x8b4d size:0x0002 C-string:'_n_VECTOR'
//         TREJECT  codep:0x1d29 wordp:0x8b5b size:0x0006 C-string:'TREJECT'
//         CLIPPER  codep:0x8b6d wordp:0x8b6d size:0x000a C-string:'CLIPPER'
//         SCANPOL  codep:0x8b83 wordp:0x8b83 size:0x000a C-string:'SCANPOL'
//         >PLANES  codep:0x8b99 wordp:0x8b99 size:0x000a C-string:'_gt_PLANES'
//           LFILL  codep:0x8bad wordp:0x8bad size:0x000a C-string:'LFILL'
//             BLT  codep:0x8bbf wordp:0x8bbf size:0x000a C-string:'BLT'
//         LFILLPO  codep:0x224c wordp:0x8bd5 size:0x000e C-string:'LFILLPO'
//           W8BE5  codep:0x224c wordp:0x8be5 size:0x0020 C-string:'W8BE5'
//         SETCLIP  codep:0x224c wordp:0x8c11 size:0x0044 C-string:'SETCLIP'
//         VDCLIPS  codep:0x224c wordp:0x8c61 size:0x0012 C-string:'VDCLIPS'
//         VCLIPSE  codep:0x224c wordp:0x8c7f size:0x0010 C-string:'VCLIPSE'
//         DCLIPSE  codep:0x224c wordp:0x8c9b size:0x0010 C-string:'DCLIPSE'
//           W8CAD  codep:0x1d29 wordp:0x8cad size:0x000c C-string:'W8CAD'
//           W8CBB  codep:0x224c wordp:0x8cbb size:0x0022 C-string:'W8CBB'
//           W8CDF  codep:0x224c wordp:0x8cdf size:0x0024 C-string:'W8CDF'
//           W8D05  codep:0x224c wordp:0x8d05 size:0x0010 C-string:'W8D05'
//         >MAINVI  codep:0x224c wordp:0x8d21 size:0x001a C-string:'_gt_MAINVI'
//         >DISPLA  codep:0x224c wordp:0x8d47 size:0x0018 C-string:'_gt_DISPLA'
//         >HIDDEN  codep:0x224c wordp:0x8d6b size:0x0018 C-string:'_gt_HIDDEN'
//           R2OVL  codep:0x2214 wordp:0x8d8d size:0x000a C-string:'R2OVL'
//           W8D99  codep:0x8d99 wordp:0x8d99 size:0x000a C-string:'W8D99'
//           W8DA5  codep:0x224c wordp:0x8da5 size:0x001e C-string:'W8DA5'
//           W8DC5  codep:0x224c wordp:0x8dc5 size:0x000c C-string:'W8DC5'
//             SRC  codep:0x1d29 wordp:0x8dd9 size:0x0002 C-string:'SRC'
//             DST  codep:0x1d29 wordp:0x8de3 size:0x0002 C-string:'DST'
//           SEGME  codep:0x1d29 wordp:0x8def size:0x0002 C-string:'SEGME'
//             PLZ  codep:0x1d29 wordp:0x8df9 size:0x0002 C-string:'PLZ'
//           XPCOL  codep:0x1d29 wordp:0x8e05 size:0x0002 C-string:'XPCOL'
//            .RAW  codep:0x224c wordp:0x8e10 size:0x0036 C-string:'DrawRAW'
//         DISPLAY  codep:0x8e52 wordp:0x8e52 size:0x0018 C-string:'DISPLAY'
//          >ALPHA  codep:0x224c wordp:0x8e75 size:0x000e C-string:'_gt_ALPHA'
//              >A  codep:0x224c wordp:0x8e8a size:0x0004 C-string:'_gt_A'
//          >LORES  codep:0x224c wordp:0x8e99 size:0x0026 C-string:'_gt_LORES'
//           W8EC1  codep:0x224c wordp:0x8ec1 size:0x0038 C-string:'W8EC1'
//           W8EFB  codep:0x224c wordp:0x8efb size:0x0016 C-string:'W8EFB'
//             D>H  codep:0x224c wordp:0x8f19 size:0x000c C-string:'D_gt_H'
//             H>D  codep:0x224c wordp:0x8f2d size:0x000c C-string:'H_gt_D'
//         SAVE-SC  codep:0x224c wordp:0x8f45 size:0x0006 C-string:'SAVE_dash_SC'
//         SCR-RES  codep:0x224c wordp:0x8f57 size:0x0006 C-string:'SCR_dash_RES'
//         V>DISPL  codep:0x224c wordp:0x8f69 size:0x003a C-string:'V_gt_DISPL'
//           W8FA5  codep:0x224c wordp:0x8fa5 size:0x0026 C-string:'W8FA5'
//           W8FCD  codep:0x224c wordp:0x8fcd size:0x0010 C-string:'W8FCD'
//         LCOPYBL  codep:0x224c wordp:0x8fe9 size:0x002e C-string:'LCOPYBL'
//           (BYE)  codep:0x224c wordp:0x9021 size:0x0018 C-string:'_ro_BYE_rc_'
//           BYE_2  codep:0x224c wordp:0x9041 size:0x0006 C-string:'BYE_2'
//           W9049  codep:0x224c wordp:0x9049 size:0x0020 C-string:'W9049'
//         SET-COL  codep:0x224c wordp:0x9075 size:0x002e C-string:'SET_dash_COL'
//       COLORMA_1  codep:0x90af wordp:0x90af size:0x002d C-string:'COLORMA_1'
//       COLORMA_2  codep:0x224c wordp:0x90e8 size:0x000a C-string:'COLORMA_2'
//         CELLCOL  codep:0x224c wordp:0x90fe size:0x0008 C-string:'CELLCOL'
//         !XYSEED  codep:0x224c wordp:0x9112 size:0x001c C-string:'StoreXYSEED'
//         SQLPLOT  codep:0x224c wordp:0x913a size:0x0014 C-string:'SQLPLOT'
//         BUFFERX  codep:0x915a wordp:0x915a size:0x002b C-string:'BUFFERX'
//           W9187  codep:0x224c wordp:0x9187 size:0x0006 C-string:'W9187'
//         .1X2CEL  codep:0x224c wordp:0x9199 size:0x000e C-string:'Draw1X2CEL'
//         TILEFIL  codep:0x224c wordp:0x91b3 size:0x0040 C-string:'TILEFIL'
//         .2X2CEL  codep:0x224c wordp:0x91ff size:0x0018 C-string:'Draw2X2CEL'
//         .4X4CEL  codep:0x224c wordp:0x9223 size:0x0012 C-string:'Draw4X4CEL'
//         .8X8CEL  codep:0x224c wordp:0x9241 size:0x0012 C-string:'Draw8X8CEL'
//         .REGION  codep:0x224c wordp:0x925f size:0x0028 C-string:'DrawREGION'
//          ?INVIS  codep:0x9292 wordp:0x9292 size:0x0037 C-string:'IsINVIS'
//         WLD>SCR  codep:0x92d5 wordp:0x92d5 size:0x0038 C-string:'WLD_gt_SCR'
//         SCR>BLT  codep:0x9319 wordp:0x9319 size:0x0014 C-string:'SCR_gt_BLT'
//         !VISWIN  codep:0x224c wordp:0x9339 size:0x0012 C-string:'StoreVISWIN'
//           W934D  codep:0x224c wordp:0x934d size:0x001c C-string:'W934D'
//         >1ICONF  codep:0x224c wordp:0x9375 size:0x0010 C-string:'_gt_1ICONF'
//         >2ICONF  codep:0x224c wordp:0x9391 size:0x0012 C-string:'_gt_2ICONF'
//         >3ICONF  codep:0x224c wordp:0x93af size:0x0012 C-string:'_gt_3ICONF'
//           W93C3  codep:0x93c3 wordp:0x93c3 size:0x0014 C-string:'W93C3'
//           W93D9  codep:0x93d9 wordp:0x93d9 size:0x0014 C-string:'W93D9'
//             !IW  codep:0x93f5 wordp:0x93f5 size:0x0014 C-string:'StoreIW'
//             !IB  codep:0x9411 wordp:0x9411 size:0x0012 C-string:'StoreIB'
//             @IX  codep:0x224c wordp:0x942b size:0x0008 C-string:'GetIX'
//             @IY  codep:0x224c wordp:0x943b size:0x0008 C-string:'GetIY'
//             @ID  codep:0x224c wordp:0x944b size:0x0008 C-string:'GetID'
//             @IC  codep:0x224c wordp:0x945b size:0x0008 C-string:'GetIC'
//             @IL  codep:0x224c wordp:0x946b size:0x0008 C-string:'GetIL'
//             @IH  codep:0x224c wordp:0x947b size:0x0008 C-string:'GetIH'
//             !IX  codep:0x224c wordp:0x948b size:0x0008 C-string:'StoreIX'
//             !IY  codep:0x224c wordp:0x949b size:0x0008 C-string:'StoreIY'
//             !ID  codep:0x224c wordp:0x94ab size:0x0008 C-string:'StoreID'
//             !IC  codep:0x224c wordp:0x94bb size:0x0008 C-string:'StoreIC'
//             !IL  codep:0x224c wordp:0x94cb size:0x0008 C-string:'StoreIL'
//             !IH  codep:0x224c wordp:0x94db size:0x0008 C-string:'StoreIH'
//         INIT-IC  codep:0x224c wordp:0x94ef size:0x000a C-string:'INIT_dash_IC'
//         .BACKGR  codep:0x224c wordp:0x9505 size:0x0006 C-string:'DrawBACKGR'
//          ?VCLIP  codep:0x224c wordp:0x9516 size:0x0030 C-string:'IsVCLIP'
//           W9548  codep:0x224c wordp:0x9548 size:0x0050 C-string:'W9548'
//           W959A  codep:0x224c wordp:0x959a size:0x0046 C-string:'W959A'
//           W95E2  codep:0x224c wordp:0x95e2 size:0x0006 C-string:'W95E2'
//       .CIRCLE_2  codep:0x224c wordp:0x95f4 size:0x0030 C-string:'DrawCIRCLE_2'
//         SYSCASE  codep:0x4b3b wordp:0x9630 size:0x0014 C-string:'SYSCASE'
//           W9646  codep:0x224c wordp:0x9646 size:0x0020 C-string:'W9646'
//           W9668  codep:0x224c wordp:0x9668 size:0x0052 C-string:'W9668'
//         .ICONCA  codep:0x4b3b wordp:0x96c6 size:0x0014 C-string:'DrawICONCA'
//         POINT>I  codep:0x224c wordp:0x96e6 size:0x0006 C-string:'POINT_gt_I'
//           .ICON  codep:0x224c wordp:0x96f6 size:0x003c C-string:'DrawICON'
//         .LOCAL-  codep:0x224c wordp:0x973e size:0x001a C-string:'DrawLOCAL_dash_'
//           W975A  codep:0x975a wordp:0x975a size:0x005e C-string:'W975A'
//       ?ICONS-_1  codep:0x224c wordp:0x97c4 size:0x0008 C-string:'IsICONS_dash__1'
//       ?ICONS-_2  codep:0x224c wordp:0x97d8 size:0x0008 C-string:'IsICONS_dash__2'
//         ?ICON=I  codep:0x224c wordp:0x97ec size:0x0068 C-string:'IsICON_eq_I'
//           W9856  codep:0x9856 wordp:0x9856 size:0x003d C-string:'W9856'
//           W9895  codep:0x224c wordp:0x9895 size:0x001e C-string:'W9895'
//           -ICON  codep:0x224c wordp:0x98bd size:0x0034 C-string:'_dash_ICON'
//           W98F3  codep:0x98f3 wordp:0x98f3 size:0x0041 C-string:'W98F3'
//         ?ICONSI  codep:0x224c wordp:0x9940 size:0x0008 C-string:'IsICONSI'
//         +ICON_2  codep:0x224c wordp:0x9952 size:0x0026 C-string:'_plus_ICON_2'
//         +ICONBO  codep:0x224c wordp:0x9984 size:0x0008 C-string:'_plus_ICONBO'
//         ORGLIST  codep:0x224c wordp:0x9998 size:0x00ce C-string:'ORGLIST'
//           SFILL  codep:0x224c wordp:0x9a70 size:0x0004 C-string:'SFILL'
//            POS.  codep:0x224c wordp:0x9a7d size:0x000a C-string:'POS_dot_'
//          WINDOW  codep:0x224c wordp:0x9a92 size:0x0038 C-string:'WINDOW'
//          .1LOGO  codep:0x224c wordp:0x9ad5 size:0x003e C-string:'Draw1LOGO'
//           W9B15  codep:0x224c wordp:0x9b15 size:0x001a C-string:'W9B15'
//           W9B31  codep:0x224c wordp:0x9b31 size:0x0058 C-string:'W9B31'
//         POLY-WI  codep:0x224c wordp:0x9b95 size:0x0008 C-string:'POLY_dash_WI'
//           >SSCT  codep:0x224c wordp:0x9ba7 size:0x002c C-string:'_gt_SSCT'
//           >TVCT  codep:0x224c wordp:0x9bdd size:0x0030 C-string:'_gt_TVCT'
//          CTPOS.  codep:0x224c wordp:0x9c18 size:0x0024 C-string:'CTPOS_dot_'
//         CTERASE  codep:0x224c wordp:0x9c48 size:0x0024 C-string:'CTERASE'
//           W9C6E  codep:0x224c wordp:0x9c6e size:0x000a C-string:'W9C6E'
//           W9C7A  codep:0x224c wordp:0x9c7a size:0x000a C-string:'W9C7A'
//           CLICK  codep:0x224c wordp:0x9c8e size:0x0018 C-string:'CLICK'
//            >SND  codep:0x224c wordp:0x9caf size:0x002a C-string:'_gt_SND'
//        BEEPON_2  codep:0x224c wordp:0x9ce4 size:0x000c C-string:'BEEPON_2'
//           W9CF2  codep:0x224c wordp:0x9cf2 size:0x0006 C-string:'W9CF2'
//           W9CFA  codep:0x224c wordp:0x9cfa size:0x0006 C-string:'W9CFA'
//           W9D02  codep:0x224c wordp:0x9d02 size:0x0006 C-string:'W9D02'
//           W9D0A  codep:0x224c wordp:0x9d0a size:0x0006 C-string:'W9D0A'
//           W9D12  codep:0x224c wordp:0x9d12 size:0x0006 C-string:'W9D12'
//           W9D1A  codep:0x224c wordp:0x9d1a size:0x0008 C-string:'W9D1A'
//           W9D24  codep:0x224c wordp:0x9d24 size:0x000a C-string:'W9D24'
//            SONG  codep:0x224c wordp:0x9d37 size:0x004a C-string:'SONG'
//            @CRS  codep:0x224c wordp:0x9d8a size:0x002c C-string:'GetCRS'
//            !CRS  codep:0x224c wordp:0x9dbf size:0x002a C-string:'StoreCRS'
//              $.  codep:0x224c wordp:0x9df0 size:0x0006 C-string:'_do__dot_'
//         POLY-ER  codep:0x224c wordp:0x9e02 size:0x0028 C-string:'POLY_dash_ER'
//         POS.PXT  codep:0x224c wordp:0x9e36 size:0x001c C-string:'POS_dot_PXT'
//           W9E54  codep:0x224c wordp:0x9e54 size:0x0020 C-string:'W9E54'
//           W9E76  codep:0x224c wordp:0x9e76 size:0x001c C-string:'W9E76'
//             WUP  codep:0x224c wordp:0x9e9a size:0x0028 C-string:'WUP'
//             WDN  codep:0x224c wordp:0x9eca size:0x0028 C-string:'WDN'
//         WLINE-U  codep:0x224c wordp:0x9efe size:0x0006 C-string:'WLINE_dash_U'
//         WLINE-D  codep:0x224c wordp:0x9f10 size:0x0006 C-string:'WLINE_dash_D'
//             GCR  codep:0x224c wordp:0x9f1e size:0x0012 C-string:'GCR'
//         WSHORTE  codep:0x224c wordp:0x9f3c size:0x0020 C-string:'WSHORTE'
//           W9F5E  codep:0x224c wordp:0x9f5e size:0x0008 C-string:'W9F5E'
//           W9F68  codep:0x224c wordp:0x9f68 size:0x0008 C-string:'W9F68'
//           >BOTT  codep:0x224c wordp:0x9f7a size:0x001c C-string:'_gt_BOTT'
//            CTCR  codep:0x224c wordp:0x9f9f size:0x0010 C-string:'CTCR'
//         TTY-SCR  codep:0x224c wordp:0x9fbb size:0x0004 C-string:'TTY_dash_SCR'
//           W9FC1  codep:0x224c wordp:0x9fc1 size:0x0008 C-string:'W9FC1'
//          CTINIT  codep:0x224c wordp:0x9fd4 size:0x0016 C-string:'CTINIT'
//            .TTY  codep:0x224c wordp:0x9ff3 size:0x0006 C-string:'DrawTTY'
//         VIEWSCR  codep:0x224c wordp:0xa005 size:0x0092 C-string:'VIEWSCR'
//         TXT-WIN  codep:0x224c wordp:0xa0a3 size:0x005e C-string:'TXT_dash_WIN'
//         AUXSCRE  codep:0x224c wordp:0xa10d size:0x0054 C-string:'AUXSCRE'
//         BTN-WIN  codep:0x224c wordp:0xa16d size:0x0054 C-string:'BTN_dash_WIN'
//         .BUTTON  codep:0x224c wordp:0xa1cd size:0x0048 C-string:'DrawBUTTON'
//            ?MRC  codep:0x224c wordp:0xa21e size:0x0030 C-string:'IsMRC'
//           WA250  codep:0x224c wordp:0xa250 size:0x001c C-string:'WA250'
//         ERASE-A  codep:0x224c wordp:0xa278 size:0x001c C-string:'ERASE_dash_A'
//         ERASE-T  codep:0x224c wordp:0xa2a0 size:0x001e C-string:'ERASE_dash_T'
//           WA2C0  codep:0x224c wordp:0xa2c0 size:0x001c C-string:'WA2C0'
//         .HIGHLI  codep:0x224c wordp:0xa2e8 size:0x001e C-string:'DrawHIGHLI'
//           WA308  codep:0x224c wordp:0xa308 size:0x002a C-string:'WA308'
//             .ON  codep:0x224c wordp:0xa33a size:0x0016 C-string:'DrawON'
//         CLR-BUT  codep:0x224c wordp:0xa35c size:0x0016 C-string:'CLR_dash_BUT'
//         INIT-BU  codep:0x224c wordp:0xa37e size:0x000e C-string:'INIT_dash_BU'
//         (SHIP-C  codep:0x224c wordp:0xa398 size:0x002a C-string:'_ro_SHIP_dash_C'
//         SHIP-CO  codep:0x224c wordp:0xa3ce size:0x0008 C-string:'SHIP_dash_CO'
//            'KEY  codep:0x224c wordp:0xa3df size:0x007e C-string:'_i_KEY'
//           WA45F  codep:0x224c wordp:0xa45f size:0x0006 C-string:'WA45F'
//           WA467  codep:0x224c wordp:0xa467 size:0x0006 C-string:'WA467'
//           WA46F  codep:0x224c wordp:0xa46f size:0x0006 C-string:'WA46F'
//           WA477  codep:0x224c wordp:0xa477 size:0x000a C-string:'WA477'
//             NOF  codep:0x1d29 wordp:0xa489 size:0x0002 C-string:'NOF'
//            FKEY  codep:0x224c wordp:0xa494 size:0x001d C-string:'FKEY'
//           WA4B3  codep:0x224c wordp:0xa4b3 size:0x0004 C-string:'WA4B3'
//             'F1  codep:0xa49a wordp:0xa4bf size:0x0002 C-string:'_i_F1'
//             'F2  codep:0xa49a wordp:0xa4c9 size:0x0002 C-string:'_i_F2'
//             'F3  codep:0xa49a wordp:0xa4d3 size:0x0002 C-string:'_i_F3'
//             'F4  codep:0xa49a wordp:0xa4dd size:0x0002 C-string:'_i_F4'
//             'F5  codep:0xa49a wordp:0xa4e7 size:0x0002 C-string:'_i_F5'
//             'F6  codep:0xa49a wordp:0xa4f1 size:0x0002 C-string:'_i_F6'
//             'F7  codep:0xa49a wordp:0xa4fb size:0x0002 C-string:'_i_F7'
//             'F8  codep:0xa49a wordp:0xa505 size:0x0002 C-string:'_i_F8'
//             'F9  codep:0xa49a wordp:0xa50f size:0x0002 C-string:'_i_F9'
//            'F10  codep:0xa49a wordp:0xa51a size:0x0002 C-string:'_i_F10'
//           WA51E  codep:0x224c wordp:0xa51e size:0x000a C-string:'WA51E'
//           WA52A  codep:0x224c wordp:0xa52a size:0x0006 C-string:'WA52A'
//           WA532  codep:0x224c wordp:0xa532 size:0x0006 C-string:'WA532'
//           WA53A  codep:0x224c wordp:0xa53a size:0x0006 C-string:'WA53A'
//           WA542  codep:0x224c wordp:0xa542 size:0x0004 C-string:'WA542'
//           WA548  codep:0x224c wordp:0xa548 size:0x0008 C-string:'WA548'
//         DOTRACE  codep:0x224c wordp:0xa55c size:0x001c C-string:'DOTRACE'
//          'TRACE  codep:0x1d29 wordp:0xa583 size:0x0002 C-string:'_i_TRACE'
//           WA587  codep:0x224c wordp:0xa587 size:0x0006 C-string:'WA587'
//         (XYSCAN  codep:0x4b3b wordp:0xa599 size:0x0074 C-string:'_ro_XYSCAN'
//          XYSCAN  codep:0x224c wordp:0xa618 size:0x000e C-string:'XYSCAN'
//           ?TRIG  codep:0x224c wordp:0xa630 size:0x0010 C-string:'IsTRIG'
//           ?QUIT  codep:0x224c wordp:0xa64a size:0x0010 C-string:'IsQUIT'
//             Y/N  codep:0x224c wordp:0xa662 size:0x0012 C-string:'Y_slash_N'
//           WA676  codep:0x224c wordp:0xa676 size:0x0012 C-string:'WA676'
//           WA68A  codep:0x224c wordp:0xa68a size:0x0006 C-string:'WA68A'
//           .ABTN  codep:0x224c wordp:0xa69a size:0x0054 C-string:'DrawABTN'
//           WA6F0  codep:0x224c wordp:0xa6f0 size:0x0020 C-string:'WA6F0'
//         .BTN-TE  codep:0x224c wordp:0xa71c size:0x004e C-string:'DrawBTN_dash_TE'
//         NEW-BUT  codep:0x224c wordp:0xa776 size:0x0044 C-string:'NEW_dash_BUT'
//         CURSORS  codep:0x224c wordp:0xa7c6 size:0x000c C-string:'CURSORS'
//         BLD-CRS  codep:0x224c wordp:0xa7de size:0x0052 C-string:'BLD_dash_CRS'
//         SET-CRS  codep:0x224c wordp:0xa83c size:0x005a C-string:'SET_dash_CRS'
//              $!  codep:0x224c wordp:0xa89d size:0x000a C-string:'_do__ex_'
//         SUBROOT  codep:0x224c wordp:0xa8b3 size:0x0008 C-string:'SUBROOT'
//         SRDEPTH  codep:0x224c wordp:0xa8c7 size:0x000a C-string:'SRDEPTH'
//           WA8D3  codep:0x224c wordp:0xa8d3 size:0x0014 C-string:'WA8D3'
//         NEXT-NO  codep:0x224c wordp:0xa8f3 size:0x001c C-string:'NEXT_dash_NO'
//         INST-VA  codep:0x7420 wordp:0xa91b size:0x0003 C-string:'INST_dash_VA'
//         INST-DA  codep:0x7420 wordp:0xa92a size:0x0003 C-string:'INST_dash_DA'
//           %NAME  codep:0x7420 wordp:0xa937 size:0x0003 C-string:'_pe_NAME'
//         ORIG-NA  codep:0x7394 wordp:0xa946 size:0x0006 C-string:'ORIG_dash_NA'
//         ELEM-NA  codep:0x7394 wordp:0xa958 size:0x0006 C-string:'ELEM_dash_NA'
//         ELEM-VA  codep:0x7394 wordp:0xa96a size:0x0006 C-string:'ELEM_dash_VA'
//         ART-NAM  codep:0x7394 wordp:0xa97c size:0x0006 C-string:'ART_dash_NAM'
//         ART-VAL  codep:0x7394 wordp:0xa98e size:0x0006 C-string:'ART_dash_VAL'
//         ART-VOL  codep:0x7394 wordp:0xa9a0 size:0x0006 C-string:'ART_dash_VOL'
//         PHR-CNT  codep:0x7420 wordp:0xa9b2 size:0x0003 C-string:'PHR_dash_CNT'
//          PHRASE  codep:0x7420 wordp:0xa9c0 size:0x0003 C-string:'PHRASE'
//          ASKING  codep:0x7420 wordp:0xa9ce size:0x0003 C-string:'ASKING'
//             U>$  codep:0x224c wordp:0xa9d9 size:0x000a C-string:'U_gt__do_'
//           WA9E5  codep:0x224c wordp:0xa9e5 size:0x0014 C-string:'WA9E5'
//           WA9FB  codep:0x224c wordp:0xa9fb size:0x0016 C-string:'WA9FB'
//         <CTVERS  codep:0x224c wordp:0xaa1d size:0x0012 C-string:'_st_CTVERS'
//         <CTASKM  codep:0x224c wordp:0xaa3b size:0x0012 C-string:'_st_CTASKM'
//         TEXT-CO  codep:0x7420 wordp:0xaa59 size:0x0003 C-string:'TEXT_dash_CO'
//         TEXT-IN  codep:0x7420 wordp:0xaa68 size:0x0003 C-string:'TEXT_dash_IN'
//         TEXT-TE  codep:0x7420 wordp:0xaa77 size:0x0003 C-string:'TEXT_dash_TE'
//         TEXT>PA  codep:0x224c wordp:0xaa86 size:0x0016 C-string:'TEXT_gt_PA'
//           CMESS  codep:0x224c wordp:0xaaa6 size:0x002c C-string:'CMESS'
//          X0MESS  codep:0x224c wordp:0xaadd size:0x002a C-string:'X0MESS'
//           0MESS  codep:0x224c wordp:0xab11 size:0x0006 C-string:'_0MESS'
//           WAB19  codep:0x2214 wordp:0xab19 size:0x0002 C-string:'WAB19'
//         -XTRAIL  codep:0x224c wordp:0xab27 size:0x002e C-string:'_dash_XTRAIL'
//           WAB57  codep:0x1d29 wordp:0xab57 size:0x0002 C-string:'WAB57'
//           SMART  codep:0x1d29 wordp:0xab63 size:0x0002 C-string:'SMART'
//           WAB67  codep:0x2214 wordp:0xab67 size:0x0002 C-string:'WAB67'
//           WAB6B  codep:0x2214 wordp:0xab6b size:0x0002 C-string:'WAB6B'
//           WAB6F  codep:0x2214 wordp:0xab6f size:0x0002 C-string:'WAB6F'
//           WAB73  codep:0x2214 wordp:0xab73 size:0x0002 C-string:'WAB73'
//           WAB77  codep:0x2214 wordp:0xab77 size:0x0002 C-string:'WAB77'
//           WAB7B  codep:0x2214 wordp:0xab7b size:0x0002 C-string:'WAB7B'
//           WAB7F  codep:0x2214 wordp:0xab7f size:0x0002 C-string:'WAB7F'
//           WAB83  codep:0x224c wordp:0xab83 size:0x000a C-string:'WAB83'
//             C+!  codep:0x224c wordp:0xab95 size:0x000e C-string:'C_plus__ex_'
//           WABA5  codep:0x224c wordp:0xaba5 size:0x004c C-string:'WABA5'
//         DISTRAC  codep:0x224c wordp:0xabfd size:0x000e C-string:'DISTRAC'
//           WAC0D  codep:0x224c wordp:0xac0d size:0x0080 C-string:'WAC0D'
//          EXPERT  codep:0x224c wordp:0xac98 size:0x0099 C-string:'EXPERT'
//             -->  codep:0x224c wordp:0xad39 size:0x0004 C-string:'_dash__dash__gt_'
//           WAD3F  codep:0x224c wordp:0xad3f size:0x0032 C-string:'WAD3F'
//           WAD73  codep:0x224c wordp:0xad73 size:0x0063 C-string:'WAD73'
//           RULE:  codep:0x224c wordp:0xade0 size:0x0095 C-string:'RULE_c_'
//         CEX+WAX  codep:0x224c wordp:0xae81 size:0x000a C-string:'CEX_plus_WAX'
//           WAE8D  codep:0x224c wordp:0xae8d size:0x0006 C-string:'WAE8D'
//           WAE95  codep:0x224c wordp:0xae95 size:0x0006 C-string:'WAE95'
//           WAE9D  codep:0x224c wordp:0xae9d size:0x0006 C-string:'WAE9D'
//           WAEA5  codep:0x224c wordp:0xaea5 size:0x0006 C-string:'WAEA5'
//           WAEAD  codep:0x224c wordp:0xaead size:0x000c C-string:'WAEAD'
//           WAEBB  codep:0x224c wordp:0xaebb size:0x0006 C-string:'WAEBB'
//           WAEC3  codep:0x224c wordp:0xaec3 size:0x0006 C-string:'WAEC3'
//           WAECB  codep:0x224c wordp:0xaecb size:0x0006 C-string:'WAECB'
//           WAED3  codep:0x224c wordp:0xaed3 size:0x000c C-string:'WAED3'
//         ?NEW-HO  codep:0x224c wordp:0xaeeb size:0x0014 C-string:'IsNEW_dash_HO'
//           WAF01  codep:0x224c wordp:0xaf01 size:0x0006 C-string:'WAF01'
//           WAF09  codep:0x224c wordp:0xaf09 size:0x006c C-string:'WAF09'
//         PARALLE  codep:0x224c wordp:0xaf81 size:0x0012 C-string:'PARALLE'
//           WAF95  codep:0x224c wordp:0xaf95 size:0x000e C-string:'WAF95'
//           WAFA5  codep:0x224c wordp:0xafa5 size:0x000e C-string:'WAFA5'
//           WAFB5  codep:0x224c wordp:0xafb5 size:0x0006 C-string:'WAFB5'
//         DOTASKS  codep:0x224c wordp:0xafc7 size:0x0060 C-string:'DOTASKS'
//         ?VCYCLE  codep:0x224c wordp:0xb033 size:0x0031 C-string:'IsVCYCLE'
//           WB066  codep:0x224c wordp:0xb066 size:0x0018 C-string:'WB066'
//           WB080  codep:0x224c wordp:0xb080 size:0x000c C-string:'WB080'
//            >XOR  codep:0x224c wordp:0xb095 size:0x002a C-string:'_gt_XOR'
//         LFCLAIM  codep:0x224c wordp:0xb0c1 size:0x0010 C-string:'LFCLAIM'
//        AINSTALL  codep:0x224c wordp:0xb0d3 size:0x0020 C-string:'AINSTALL'
//        BINSTALL  codep:0x224c wordp:0xb0f5 size:0x0008 C-string:'BINSTALL'
//       AINSTALLS  codep:0x224c wordp:0xb0ff size:0x0024 C-string:'AINSTALLS'
//           WB125  codep:0x224c wordp:0xb125 size:0x001a C-string:'WB125'
//       BINSTALLS  codep:0x224c wordp:0xb141 size:0x0088 C-string:'BINSTALLS'
//          ICINIT  codep:0x224c wordp:0xb1d4 size:0x002a C-string:'ICINIT'
//           WB200  codep:0x224c wordp:0xb200 size:0x001e C-string:'WB200'
//         XKEYINT  codep:0xb22a wordp:0xb22a size:0x001f C-string:'XKEYINT'
//         INSTALL  codep:0x224c wordp:0xb255 size:0x0024 C-string:'INSTALL'
//           WB27B  codep:0x224c wordp:0xb27b size:0x000e C-string:'WB27B'
//           WB28B  codep:0x224c wordp:0xb28b size:0x000a C-string:'WB28B'
//         RESTORE  codep:0x224c wordp:0xb2a1 size:0x0018 C-string:'RESTORE'
//           WB2BB  codep:0x224c wordp:0xb2bb size:0x0006 C-string:'WB2BB'
//           WB2C3  codep:0x224c wordp:0xb2c3 size:0x0022 C-string:'WB2C3'
//           WB2E7  codep:0x224c wordp:0xb2e7 size:0x000a C-string:'WB2E7'
//         LOAD-DI  codep:0x224c wordp:0xb2fd size:0x0020 C-string:'LOAD_dash_DI'
//          LDSILS  codep:0x224c wordp:0xb328 size:0x000c C-string:'LDSILS'
//            !PAL  codep:0x224c wordp:0xb33d size:0x0012 C-string:'StorePAL'
//         CONFIGU  codep:0x224c wordp:0xb35b size:0x006c C-string:'CONFIGU'
//         STARTER  codep:0x224c wordp:0xb3d3 size:0x000a C-string:'STARTER'
//             DOS  codep:0x224c wordp:0xb3e5 size:0x0002 C-string:'DOS'
//           +EDIT  codep:0x1ab5 wordp:0xb3f1 size:0x0014 C-string:'_plus_EDIT'
//         EDIT-OV  codep:0x83dd wordp:0xb411 size:0x0002 C-string:'EDIT_dash_OV'
//            EDIT  codep:0x224c wordp:0xb41c size:0x0008 C-string:'EDIT'
//            USE"  codep:0x224c wordp:0xb42d size:0x0006 C-string:'USE_qm_'
//              SB  codep:0x2214 wordp:0xb43a size:0x0002 C-string:'SB'
//           WB43E  codep:0xb43e wordp:0xb43e size:0x0009 C-string:'WB43E'
//           WB449  codep:0x224c wordp:0xb449 size:0x000e C-string:'WB449'
//           WB459  codep:0x224c wordp:0xb459 size:0x0018 C-string:'WB459'
//           WB473  codep:0x224c wordp:0xb473 size:0x0008 C-string:'WB473'
//           WB47D  codep:0x224c wordp:0xb47d size:0x000c C-string:'WB47D'
//           WB48B  codep:0x224c wordp:0xb48b size:0x0008 C-string:'WB48B'
//           WB495  codep:0x224c wordp:0xb495 size:0x0012 C-string:'WB495'
//           WB4A9  codep:0x224c wordp:0xb4a9 size:0x0018 C-string:'WB4A9'
//           WB4C3  codep:0x224c wordp:0xb4c3 size:0x0014 C-string:'WB4C3'
//           WB4D9  codep:0x224c wordp:0xb4d9 size:0x0058 C-string:'WB4D9'
//           WB533  codep:0x224c wordp:0xb533 size:0x001e C-string:'WB533'
//              WS  codep:0x224c wordp:0xb558 size:0x0006 C-string:'WS'
//              WL  codep:0x224c wordp:0xb565 size:0x0006 C-string:'WL'
//           WB56D  codep:0x224c wordp:0xb56d size:0x0014 C-string:'WB56D'
//              .S  codep:0x224c wordp:0xb588 size:0x0010 C-string:'DrawS'
//           MUSIC  codep:0x1ab5 wordp:0xb5a2 size:0x0014 C-string:'MUSIC'
//        MUSIC-OV  codep:0x83dd wordp:0xb5c3 size:0x0002 C-string:'MUSIC_dash_OV'
//          INTROS  codep:0x224c wordp:0xb5d0 size:0x0006 C-string:'INTROS'
//           HIMUS  codep:0x224c wordp:0xb5e0 size:0x0006 C-string:'HIMUS'
//          TALLOC  codep:0x224c wordp:0xb5f1 size:0x0006 C-string:'TALLOC'
//         SETABLT  codep:0x224c wordp:0xb603 size:0x0006 C-string:'SETABLT'
//             *OP  codep:0x3b74 wordp:0xb611 size:0x0004 C-string:'_star_OP'
//           *COMM  codep:0x3b74 wordp:0xb61f size:0x0004 C-string:'_star_COMM'
//            *EOL  codep:0x3b74 wordp:0xb62c size:0x0004 C-string:'_star_EOL'
//           OTABL  codep:0x1d29 wordp:0xb63a size:0x0036 C-string:'OTABL'
//            +ORG  codep:0x4b3b wordp:0xb679 size:0x001c C-string:'_plus_ORG'
//           ORGIA  codep:0x224c wordp:0xb69f size:0x0018 C-string:'ORGIA'
//        MISC-VOC  codep:0x1ab5 wordp:0xb6c1 size:0x0014 C-string:'MISC_dash_VOC'
//         MISC-OV  codep:0x83dd wordp:0xb6df size:0x0002 C-string:'MISC_dash_OV'
//          =CARGO  codep:0x224c wordp:0xb6ec size:0x0006 C-string:'_eq_CARGO'
//          OV#IN$  codep:0x224c wordp:0xb6fd size:0x0006 C-string:'OV_n_IN_do_'
//          OVQUIT  codep:0x224c wordp:0xb70e size:0x0006 C-string:'OVQUIT'
//          OV.0$$  codep:0x224c wordp:0xb71f size:0x0006 C-string:'OV_dot_0_do__do_'
//          OV0$$$  codep:0x224c wordp:0xb730 size:0x0006 C-string:'OV0_do__do__do_'
//        OVTRIM_2  codep:0x224c wordp:0xb741 size:0x0006 C-string:'OVTRIM_2'
//        OVTRIM_1  codep:0x224c wordp:0xb752 size:0x000e C-string:'OVTRIM_1'
//          OV?ART  codep:0x224c wordp:0xb76b size:0x0006 C-string:'OV_ask_ART'
//          OV?.EQ  codep:0x224c wordp:0xb77c size:0x0006 C-string:'OV_ask__dot_EQ'
//          TV?ART  codep:0x224c wordp:0xb78d size:0x0006 C-string:'TV_ask_ART'
//            RSTE  codep:0x224c wordp:0xb79c size:0x0006 C-string:'RSTE'
//          SCON-V  codep:0x1ab5 wordp:0xb7ad size:0x0014 C-string:'SCON_dash_V'
//          SCON-O  codep:0x83dd wordp:0xb7cc size:0x0002 C-string:'SCON_dash_O'
//             TOW  codep:0x224c wordp:0xb7d6 size:0x0006 C-string:'TOW'
//          .NOFUN  codep:0x224c wordp:0xb7e7 size:0x0006 C-string:'DrawNOFUN'
//          FX-VOC  codep:0x1ab5 wordp:0xb7f8 size:0x0014 C-string:'FX_dash_VOC'
//           FX-OV  codep:0x83dd wordp:0xb816 size:0x0002 C-string:'FX_dash_OV'
//           PSYCH  codep:0x224c wordp:0xb822 size:0x0008 C-string:'PSYCH'
//           .TMAP  codep:0x224c wordp:0xb834 size:0x0006 C-string:'DrawTMAP'
//          IT-VOC  codep:0x1ab5 wordp:0xb845 size:0x0014 C-string:'IT_dash_VOC'
//           IT-OV  codep:0x83dd wordp:0xb863 size:0x0002 C-string:'IT_dash_OV'
//           WB867  codep:0x224c wordp:0xb867 size:0x0006 C-string:'WB867'
//           WB86F  codep:0x224c wordp:0xb86f size:0x0006 C-string:'WB86F'
//           WB877  codep:0x224c wordp:0xb877 size:0x0006 C-string:'WB877'
//           WB87F  codep:0x224c wordp:0xb87f size:0x0006 C-string:'WB87F'
//           WB887  codep:0x224c wordp:0xb887 size:0x0006 C-string:'WB887'
//           WB88F  codep:0x224c wordp:0xb88f size:0x0006 C-string:'WB88F'
//           WB897  codep:0x224c wordp:0xb897 size:0x0006 C-string:'WB897'
//           WB89F  codep:0x224c wordp:0xb89f size:0x0006 C-string:'WB89F'
//           WB8A7  codep:0x224c wordp:0xb8a7 size:0x0006 C-string:'WB8A7'
//           WB8AF  codep:0x224c wordp:0xb8af size:0x0006 C-string:'WB8AF'
//           WB8B7  codep:0x224c wordp:0xb8b7 size:0x0006 C-string:'WB8B7'
//         DATE$>A  codep:0x224c wordp:0xb8c9 size:0x0008 C-string:'DATE_do__gt_A'
//         BOX>LIS  codep:0x224c wordp:0xb8dd size:0x0008 C-string:'BOX_gt_LIS'
//         GET-BOX  codep:0x224c wordp:0xb8f1 size:0x0008 C-string:'GET_dash_BOX'
//         MAKE-SC  codep:0x224c wordp:0xb905 size:0x0008 C-string:'MAKE_dash_SC'
//         DELETE-  codep:0x224c wordp:0xb919 size:0x0008 C-string:'DELETE_dash_'
//         BOX>TOC  codep:0x224c wordp:0xb92d size:0x0008 C-string:'BOX_gt_TOC'
//            >BOX  codep:0x224c wordp:0xb93e size:0x0008 C-string:'_gt_BOX'
//            BOX>  codep:0x224c wordp:0xb94f size:0x0008 C-string:'BOX_gt_'
//          (BOX>)  codep:0x224c wordp:0xb962 size:0x0008 C-string:'_ro_BOX_gt__rc_'
//         CLASS>B  codep:0x224c wordp:0xb976 size:0x0008 C-string:'CLASS_gt_B'
//         ITEM>PA  codep:0x224c wordp:0xb98a size:0x0008 C-string:'ITEM_gt_PA'
//         BLT-VOC  codep:0x1ab5 wordp:0xb99e size:0x0014 C-string:'BLT_dash_VOC'
//            BLT-  codep:0x83dd wordp:0xb9bb size:0x0002 C-string:'BLT_dash_'
//          @.HYBR  codep:0x224c wordp:0xb9c8 size:0x0006 C-string:'Get_dot_HYBR'
//           .TPIC  codep:0x224c wordp:0xb9d8 size:0x0006 C-string:'DrawTPIC'
//           .APIC  codep:0x224c wordp:0xb9e8 size:0x0006 C-string:'DrawAPIC'
//          SETBLT  codep:0x224c wordp:0xb9f9 size:0x0006 C-string:'SETBLT'
//          LDAPIC  codep:0x224c wordp:0xba0a size:0x0006 C-string:'LDAPIC'
//           LDSYS  codep:0x224c wordp:0xba1a size:0x0006 C-string:'LDSYS'
//          VICONP  codep:0x1ab5 wordp:0xba2b size:0x0014 C-string:'VICONP'
//           ICONP  codep:0x83dd wordp:0xba49 size:0x0002 C-string:'ICONP'
//           ICON-  codep:0x224c wordp:0xba55 size:0x0006 C-string:'ICON_dash_'
//         +ICON_1  codep:0x224c wordp:0xba65 size:0x0006 C-string:'_plus_ICON_1'
//           FRACT  codep:0x1ab5 wordp:0xba75 size:0x0014 C-string:'FRACT'
//        FRACT-OV  codep:0x83dd wordp:0xba96 size:0x0002 C-string:'FRACT_dash_OV'
//           MERCA  codep:0x224c wordp:0xbaa2 size:0x0006 C-string:'MERCA'
//           NEWCO  codep:0x224c wordp:0xbab2 size:0x0006 C-string:'NEWCO'
//           MAKE-  codep:0x224c wordp:0xbac2 size:0x0006 C-string:'MAKE_dash_'
//           SCRLC  codep:0x224c wordp:0xbad2 size:0x0006 C-string:'SCRLC'
//          BTADDR  codep:0x1d29 wordp:0xbae3 size:0x0002 C-string:'BTADDR'
// HUFF_BT_POINTER  codep:0x1d29 wordp:0xbae7 size:0x0002 C-string:'HUFF_BT_POINTER'
// HUFF_ACTIVE_POINTER_BYTE  codep:0x1d29 wordp:0xbaeb size:0x0002 C-string:'HUFF_ACTIVE_POINTER_BYTE'
//           WBAEF  codep:0x1d29 wordp:0xbaef size:0x0002 C-string:'WBAEF'
// HUFF_ACTIVE_POINTER_BIT  codep:0x1d29 wordp:0xbaf3 size:0x0002 C-string:'HUFF_ACTIVE_POINTER_BIT'
// HUFF_PHRASE_ADDCHAR  codep:0x2214 wordp:0xbaf7 size:0x0002 C-string:'HUFF_PHRASE_ADDCHAR'
// HUFF_PHRASE_INIT  codep:0x2214 wordp:0xbafb size:0x0002 C-string:'HUFF_PHRASE_INIT'
// HUFF_PHRASE_FINISH  codep:0x2214 wordp:0xbaff size:0x0002 C-string:'HUFF_PHRASE_FINISH'
// HUFF_MOVE_ACTIVE_POINTER  codep:0x224c wordp:0xbb03 size:0x0024 C-string:'HUFF_MOVE_ACTIVE_POINTER'
//           WBB29  codep:0x224c wordp:0xbb29 size:0x004e C-string:'WBB29'
//           HUFF>  codep:0x224c wordp:0xbb81 size:0x0040 C-string:'HUFF_gt_'
//         PHRASE$  codep:0x7420 wordp:0xbbcd size:0x0003 C-string:'PHRASE_do_'
// HUFF_LSCAN_INIT  codep:0x224c wordp:0xbbd2 size:0x0006 C-string:'HUFF_LSCAN_INIT'
// HUFF_LSCAN_ADDCHAR  codep:0x224c wordp:0xbbda size:0x0014 C-string:'HUFF_LSCAN_ADDCHAR'
// HUFF_LSCAN_FINISH  codep:0x224c wordp:0xbbf0 size:0x0072 C-string:'HUFF_LSCAN_FINISH'
//         ENG-VOC  codep:0x1ab5 wordp:0xbc6e size:0x0014 C-string:'ENG_dash_VOC'
//           ENG-O  codep:0x83dd wordp:0xbc8c size:0x0002 C-string:'ENG_dash_O'
//           OV/DA  codep:0x224c wordp:0xbc98 size:0x0006 C-string:'OV_slash_DA'
//           OV/RE  codep:0x224c wordp:0xbca8 size:0x0006 C-string:'OV_slash_RE'
//           OVDBA  codep:0x224c wordp:0xbcb8 size:0x0006 C-string:'OVDBA'
//          SETTER  codep:0x1ab5 wordp:0xbcc9 size:0x0014 C-string:'SETTER'
//        PLSET-OV  codep:0x83dd wordp:0xbcea size:0x0002 C-string:'PLSET_dash_OV'
//        SET-PLAN  codep:0x224c wordp:0xbcf9 size:0x0006 C-string:'SET_dash_PLAN'
//         ORBIT_2  codep:0x1ab5 wordp:0xbd09 size:0x0014 C-string:'ORBIT_2'
//         ORBIT_1  codep:0x83dd wordp:0xbd27 size:0x0002 C-string:'ORBIT_1'
//           INIT-  codep:0x224c wordp:0xbd33 size:0x0008 C-string:'INIT_dash_'
//           SHOWS  codep:0x224c wordp:0xbd45 size:0x0006 C-string:'SHOWS'
//           DESCE  codep:0x224c wordp:0xbd55 size:0x0008 C-string:'DESCE'
//           ASCEN  codep:0x224c wordp:0xbd67 size:0x0008 C-string:'ASCEN'
//           ROTAT  codep:0x224c wordp:0xbd79 size:0x0008 C-string:'ROTAT'
//           SEEGR  codep:0x224c wordp:0xbd8b size:0x0006 C-string:'SEEGR'
//           MROTA  codep:0x224c wordp:0xbd9b size:0x0008 C-string:'MROTA'
//         CULTURE  codep:0x1ab5 wordp:0xbdaf size:0x0014 C-string:'CULTURE'
//          CULTUR  codep:0x83dd wordp:0xbdce size:0x0002 C-string:'CULTUR'
//           .CULT  codep:0x224c wordp:0xbdda size:0x0006 C-string:'DrawCULT'
//           .RACE  codep:0x224c wordp:0xbdea size:0x0006 C-string:'DrawRACE'
//          DESCRI  codep:0x1ab5 wordp:0xbdfb size:0x0014 C-string:'DESCRI'
//           DESCR  codep:0x83dd wordp:0xbe19 size:0x0002 C-string:'DESCR'
//           DO-LO  codep:0x224c wordp:0xbe25 size:0x0006 C-string:'DO_dash_LO'
//           .LIFE  codep:0x224c wordp:0xbe35 size:0x0006 C-string:'DrawLIFE'
//           .HUFF  codep:0x224c wordp:0xbe45 size:0x0006 C-string:'DrawHUFF'
//           .AHUF  codep:0x224c wordp:0xbe55 size:0x0006 C-string:'DrawAHUF'
//            SSCN  codep:0x224c wordp:0xbe64 size:0x0006 C-string:'SSCN'
//           ?COMS  codep:0x224c wordp:0xbe74 size:0x0006 C-string:'IsCOMS'
//           SSCAN  codep:0x224c wordp:0xbe84 size:0x0008 C-string:'SSCAN'
//           CPAUS  codep:0x224c wordp:0xbe96 size:0x0006 C-string:'CPAUS'
//        COMM-VOC  codep:0x1ab5 wordp:0xbea9 size:0x0014 C-string:'COMM_dash_VOC'
//         COMM-OV  codep:0x83dd wordp:0xbec9 size:0x0002 C-string:'COMM_dash_OV'
//       OVINIT-_2  codep:0x224c wordp:0xbed7 size:0x0010 C-string:'OVINIT_dash__2'
//          U-COMM  codep:0x224c wordp:0xbef2 size:0x0008 C-string:'U_dash_COMM'
//         COMM-EX  codep:0x224c wordp:0xbf06 size:0x0008 C-string:'COMM_dash_EX'
//             A>O  codep:0x224c wordp:0xbf16 size:0x0008 C-string:'A_gt_O'
//             O>C  codep:0x224c wordp:0xbf26 size:0x0008 C-string:'O_gt_C'
//           >HAIL  codep:0x224c wordp:0xbf38 size:0x0006 C-string:'_gt_HAIL'
//         OV+!EDL  codep:0x224c wordp:0xbf4a size:0x0008 C-string:'OV_plus__ex_EDL'
//         OVEDL+A  codep:0x224c wordp:0xbf5e size:0x0008 C-string:'OVEDL_plus_A'
//            P>CT  codep:0x224c wordp:0xbf6f size:0x0008 C-string:'P_gt_CT'
//            CTUP  codep:0x224c wordp:0xbf80 size:0x0006 C-string:'CTUP'
//            1SYL  codep:0x224c wordp:0xbf8f size:0x0006 C-string:'_1SYL'
//         MCOMM-E  codep:0x224c wordp:0xbfa1 size:0x0008 C-string:'MCOMM_dash_E'
//         MEDL+AU  codep:0x224c wordp:0xbfb5 size:0x0008 C-string:'MEDL_plus_AU'
//          DA-STR  codep:0x224c wordp:0xbfc8 size:0x0008 C-string:'DA_dash_STR'
//           EDL>P  codep:0x224c wordp:0xbfda size:0x0006 C-string:'EDL_gt_P'
//       BLDLI-VOC  codep:0x1ab5 wordp:0xbfea size:0x0014 C-string:'BLDLI_dash_VOC'
//        BLDLI-OV  codep:0x83dd wordp:0xc008 size:0x0002 C-string:'BLDLI_dash_OV'
//           BLDLI  codep:0x224c wordp:0xc014 size:0x0006 C-string:'BLDLI'
//           SET-C  codep:0x224c wordp:0xc024 size:0x0006 C-string:'SET_dash_C'
//             T>O  codep:0x224c wordp:0xc032 size:0x0006 C-string:'T_gt_O'
//           !TAMT  codep:0x224c wordp:0xc042 size:0x0006 C-string:'StoreTAMT'
//           @TAMT  codep:0x224c wordp:0xc052 size:0x0006 C-string:'GetTAMT'
//         SCI-VOC  codep:0x1ab5 wordp:0xc064 size:0x0014 C-string:'SCI_dash_VOC'
//          SCI-OV  codep:0x83dd wordp:0xc083 size:0x0002 C-string:'SCI_dash_OV'
//          OV/STA  codep:0x224c wordp:0xc090 size:0x0006 C-string:'OV_slash_STA'
//          OV/STX  codep:0x224c wordp:0xc0a1 size:0x0006 C-string:'OV_slash_STX'
//          OVSTAR  codep:0x224c wordp:0xc0b2 size:0x0008 C-string:'OVSTAR'
//          OV/SEN  codep:0x224c wordp:0xc0c5 size:0x0006 C-string:'OV_slash_SEN'
//          ?HEAVY  codep:0x224c wordp:0xc0d6 size:0x0006 C-string:'IsHEAVY'
//          /STARD  codep:0x224c wordp:0xc0e7 size:0x0008 C-string:'_slash_STARD'
//           .SORD  codep:0x224c wordp:0xc0f9 size:0x0024 C-string:'DrawSORD'
//          HEALTI  codep:0x1d29 wordp:0xc128 size:0x0002 C-string:'HEALTI'
//          LASTAP  codep:0x1d29 wordp:0xc135 size:0x0004 C-string:'LASTAP'
//          ROSTER  codep:0x1d29 wordp:0xc144 size:0x0012 C-string:'ROSTER'
//          HEALER  codep:0x1ab5 wordp:0xc161 size:0x0014 C-string:'HEALER'
//          HEAL-O  codep:0x83dd wordp:0xc180 size:0x0002 C-string:'HEAL_dash_O'
//            (OBI  codep:0x224c wordp:0xc18b size:0x0006 C-string:'_ro_OBI'
//            (.VI  codep:0x224c wordp:0xc19a size:0x0006 C-string:'_ro__dot_VI'
//            (HEA  codep:0x224c wordp:0xc1a9 size:0x0006 C-string:'_ro_HEA'
//            (ROL  codep:0x224c wordp:0xc1b8 size:0x0006 C-string:'_ro_ROL'
//            HEAL  codep:0x224c wordp:0xc1c7 size:0x0008 C-string:'HEAL'
//            .VIT  codep:0x224c wordp:0xc1d8 size:0x0008 C-string:'DrawVIT'
//            KILL  codep:0x224c wordp:0xc1e9 size:0x0006 C-string:'KILL'
//            ?APP  codep:0x224c wordp:0xc1f8 size:0x002e C-string:'IsAPP'
//        BANK-VOC  codep:0x1ab5 wordp:0xc233 size:0x0014 C-string:'BANK_dash_VOC'
//         BANK-OV  codep:0x83dd wordp:0xc253 size:0x0002 C-string:'BANK_dash_OV'
//       OVINIT-_1  codep:0x224c wordp:0xc261 size:0x0006 C-string:'OVINIT_dash__1'
//         OVTRANS  codep:0x224c wordp:0xc273 size:0x0006 C-string:'OVTRANS'
//         OVD@BAL  codep:0x224c wordp:0xc285 size:0x0006 C-string:'OVD_at_BAL'
//         OVD!BAL  codep:0x224c wordp:0xc297 size:0x0006 C-string:'OVD_ex_BAL'
//         OV?BALA  codep:0x224c wordp:0xc2a9 size:0x0006 C-string:'OV_ask_BALA'
//         OV!TFLA  codep:0x224c wordp:0xc2bb size:0x0006 C-string:'OV_ex_TFLA'
//         I-TRANS  codep:0x224c wordp:0xc2cd size:0x0006 C-string:'I_dash_TRANS'
//             U-B  codep:0x224c wordp:0xc2db size:0x0006 C-string:'U_dash_B'
//         T+BALAN  codep:0x224c wordp:0xc2ed size:0x000c C-string:'T_plus_BALAN'
//        XCOM-VOC  codep:0x1ab5 wordp:0xc306 size:0x0014 C-string:'XCOM_dash_VOC'
//         XCOM-OV  codep:0x83dd wordp:0xc326 size:0x0002 C-string:'XCOM_dash_OV'
//           XCOMM  codep:0x224c wordp:0xc332 size:0x0006 C-string:'XCOMM'
//           #>PRO  codep:0x224c wordp:0xc342 size:0x0006 C-string:'_n__gt_PRO'
//          REPAIR  codep:0x1ab5 wordp:0xc353 size:0x0014 C-string:'REPAIR'
//         REPAIR-  codep:0x83dd wordp:0xc373 size:0x0002 C-string:'REPAIR_dash_'
//         DO-REPA  codep:0x224c wordp:0xc381 size:0x0010 C-string:'DO_dash_REPA'
//         ?SHIP-R  codep:0x224c wordp:0xc39d size:0x0026 C-string:'IsSHIP_dash_R'
//         MREPAIR  codep:0x224c wordp:0xc3cf size:0x0020 C-string:'MREPAIR'
//           BARTE  codep:0x1ab5 wordp:0xc3f9 size:0x0014 C-string:'BARTE'
//         BARTER-  codep:0x83dd wordp:0xc419 size:0x0002 C-string:'BARTER_dash_'
//         DOENTER  codep:0x224c wordp:0xc427 size:0x0008 C-string:'DOENTER'
//          DOOPEN  codep:0x224c wordp:0xc43a size:0x0008 C-string:'DOOPEN'
//         DOBARTE  codep:0x224c wordp:0xc44e size:0x0008 C-string:'DOBARTE'
//         !PFLAGS  codep:0x224c wordp:0xc462 size:0x0008 C-string:'StorePFLAGS'
//             L>P  codep:0x224c wordp:0xc472 size:0x0008 C-string:'L_gt_P'
//             MAR  codep:0x1ab5 wordp:0xc482 size:0x0014 C-string:'MAR'
//           MARKE  codep:0x83dd wordp:0xc4a0 size:0x0002 C-string:'MARKE'
//           TRADE  codep:0x224c wordp:0xc4ac size:0x0008 C-string:'TRADE'
//        SITE-VOC  codep:0x1ab5 wordp:0xc4c1 size:0x0014 C-string:'SITE_dash_VOC'
//         SITE-OV  codep:0x83dd wordp:0xc4e1 size:0x0002 C-string:'SITE_dash_OV'
//         (.MERC)  codep:0x224c wordp:0xc4ef size:0x0008 C-string:'_ro__dot_MERC_rc_'
//       (GETSITE)  codep:0x224c wordp:0xc505 size:0x000a C-string:'_ro_GETSITE_rc_'
//        FLUX-VOC  codep:0x1ab5 wordp:0xc51c size:0x0014 C-string:'FLUX_dash_VOC'
//         FLUX-OV  codep:0x83dd wordp:0xc53c size:0x0002 C-string:'FLUX_dash_OV'
//           OVFLU  codep:0x224c wordp:0xc548 size:0x0006 C-string:'OVFLU'
//           JUMPF  codep:0x224c wordp:0xc558 size:0x0006 C-string:'JUMPF'
//       DPART-VOC  codep:0x1ab5 wordp:0xc56c size:0x0014 C-string:'DPART_dash_VOC'
//        DPART-OV  codep:0x83dd wordp:0xc58d size:0x0002 C-string:'DPART_dash_OV'
//          DEPART  codep:0x224c wordp:0xc59a size:0x0006 C-string:'DEPART'
//         CLOUD-V  codep:0x1ab5 wordp:0xc5ac size:0x0014 C-string:'CLOUD_dash_V'
//         CLOUD-O  codep:0x83dd wordp:0xc5cc size:0x0002 C-string:'CLOUD_dash_O'
//         DO-CLOU  codep:0x224c wordp:0xc5da size:0x0006 C-string:'DO_dash_CLOU'
//         NAV-VOC  codep:0x1ab5 wordp:0xc5ec size:0x0014 C-string:'NAV_dash_VOC'
//           NAV-O  codep:0x83dd wordp:0xc60a size:0x0002 C-string:'NAV_dash_O'
//           OV/(D  codep:0x224c wordp:0xc616 size:0x0006 C-string:'OV_slash__ro_D'
//           OV/(U  codep:0x224c wordp:0xc626 size:0x0006 C-string:'OV_slash__ro_U'
//           OV-SH  codep:0x224c wordp:0xc636 size:0x000e C-string:'OV_dash_SH'
//           OV-AR  codep:0x224c wordp:0xc64e size:0x000e C-string:'OV_dash_AR'
//      DAMAGE-VOC  codep:0x1ab5 wordp:0xc668 size:0x0014 C-string:'DAMAGE_dash_VOC'
//       DAMAGE-OV  codep:0x83dd wordp:0xc688 size:0x0002 C-string:'DAMAGE_dash_OV'
//           DO-DA  codep:0x224c wordp:0xc694 size:0x0008 C-string:'DO_dash_DA'
//            >DAM  codep:0x224c wordp:0xc6a5 size:0x000c C-string:'_gt_DAM'
//            .AUX  codep:0x224c wordp:0xc6ba size:0x0006 C-string:'DrawAUX'
//           CREWD  codep:0x224c wordp:0xc6ca size:0x0006 C-string:'CREWD'
//            BLST  codep:0x224c wordp:0xc6d9 size:0x0006 C-string:'BLST'
//           #MISS  codep:0x1d29 wordp:0xc6e9 size:0x0002 C-string:'_n_MISS'
//            WEAP  codep:0x1ab5 wordp:0xc6f4 size:0x0014 C-string:'WEAP'
//             WEA  codep:0x83dd wordp:0xc710 size:0x0002 C-string:'WEA'
//             TAR  codep:0x224c wordp:0xc71a size:0x0006 C-string:'TAR'
//           DTA_2  codep:0x224c wordp:0xc728 size:0x0008 C-string:'DTA_2'
//           DNL_2  codep:0x224c wordp:0xc738 size:0x0006 C-string:'DNL_2'
//             GNL  codep:0x224c wordp:0xc746 size:0x0006 C-string:'GNL'
//           DNL_1  codep:0x224c wordp:0xc754 size:0x0008 C-string:'DNL_1'
//         EYE-VOC  codep:0x1ab5 wordp:0xc768 size:0x0014 C-string:'EYE_dash_VOC'
//           EYE-O  codep:0x83dd wordp:0xc786 size:0x0002 C-string:'EYE_dash_O'
//              AI  codep:0x224c wordp:0xc78f size:0x0008 C-string:'AI'
//      COMBAU-VOC  codep:0x1ab5 wordp:0xc7a2 size:0x0014 C-string:'COMBAU_dash_VOC'
//       COMBAU-OV  codep:0x83dd wordp:0xc7c1 size:0x0002 C-string:'COMBAU_dash_OV'
//          CSCALE  codep:0x224c wordp:0xc7ce size:0x0006 C-string:'CSCALE'
//             @NF  codep:0x224c wordp:0xc7dc size:0x0006 C-string:'GetNF'
//      COMBAT-VOC  codep:0x1ab5 wordp:0xc7f1 size:0x0014 C-string:'COMBAT_dash_VOC'
//        COMBAT-O  codep:0x83dd wordp:0xc812 size:0x0002 C-string:'COMBAT_dash_O'
//            (WAR  codep:0x224c wordp:0xc81d size:0x0008 C-string:'_ro_WAR'
//            (COM  codep:0x224c wordp:0xc82e size:0x0006 C-string:'_ro_COM'
//             C&C  codep:0x224c wordp:0xc83c size:0x0008 C-string:'C_and_C'
//              ?J  codep:0x224c wordp:0xc84b size:0x0006 C-string:'IsJ'
//            ?JMP  codep:0x224c wordp:0xc85a size:0x0008 C-string:'IsJMP'
//            COMB  codep:0x224c wordp:0xc86b size:0x0008 C-string:'COMB'
//             WAR  codep:0x224c wordp:0xc87b size:0x0008 C-string:'WAR'
//            END-  codep:0x1ab5 wordp:0xc88c size:0x0014 C-string:'END_dash_'
//            ENDI  codep:0x83dd wordp:0xc8a9 size:0x0002 C-string:'ENDI'
//            HALL  codep:0x224c wordp:0xc8b4 size:0x0006 C-string:'HALL'
//            ?FRE  codep:0x224c wordp:0xc8c3 size:0x0006 C-string:'IsFRE'
//             DHL  codep:0x224c wordp:0xc8d1 size:0x0006 C-string:'DHL'
//            DUHL  codep:0x224c wordp:0xc8e0 size:0x0008 C-string:'DUHL'
//            BOLT  codep:0x224c wordp:0xc8f1 size:0x0006 C-string:'BOLT'
//            IUHL  codep:0x224c wordp:0xc900 size:0x0006 C-string:'IUHL'
//       HMISC-VOC  codep:0x1ab5 wordp:0xc914 size:0x0014 C-string:'HMISC_dash_VOC'
//         HMISC-O  codep:0x83dd wordp:0xc934 size:0x0002 C-string:'HMISC_dash_O'
//             UFM  codep:0x224c wordp:0xc93e size:0x0006 C-string:'UFM'
//             USM  codep:0x224c wordp:0xc94c size:0x0006 C-string:'USM'
//             NFM  codep:0x224c wordp:0xc95a size:0x0006 C-string:'NFM'
//             NSM  codep:0x224c wordp:0xc968 size:0x0006 C-string:'NSM'
//             .EX  codep:0x224c wordp:0xc976 size:0x0006 C-string:'DrawEX'
//            DMSG  codep:0x224c wordp:0xc985 size:0x0006 C-string:'DMSG'
//            D-UP  codep:0x224c wordp:0xc994 size:0x0006 C-string:'D_dash_UP'
//            ORB>  codep:0x224c wordp:0xc9a3 size:0x0006 C-string:'ORB_gt_'
//            >ORB  codep:0x224c wordp:0xc9b2 size:0x0006 C-string:'_gt_ORB'
//            OSET  codep:0x224c wordp:0xc9c1 size:0x0006 C-string:'OSET'
//           ORBUP  codep:0x224c wordp:0xc9d1 size:0x0008 C-string:'ORBUP'
//         TRAK-EN  codep:0x224c wordp:0xc9e5 size:0x001e C-string:'TRAK_dash_EN'
//           !'EXT  codep:0x224c wordp:0xca0d size:0x0006 C-string:'Store_i_EXT'
//          (CTXT)  codep:0x224c wordp:0xca1e size:0x0006 C-string:'_ro_CTXT_rc_'
//       CONTEXT_2  codep:0x224c wordp:0xca30 size:0x0008 C-string:'CONTEXT_2'
//         +A-VESS  codep:0x224c wordp:0xca44 size:0x0006 C-string:'_plus_A_dash_VESS'
//         MTRAK-E  codep:0x224c wordp:0xca56 size:0x0008 C-string:'MTRAK_dash_E'
//         SHPMOV-  codep:0x1ab5 wordp:0xca6a size:0x0014 C-string:'SHPMOV_dash_'
//           HYPER  codep:0x83dd wordp:0xca88 size:0x0002 C-string:'HYPER'
//             FLY  codep:0x224c wordp:0xca92 size:0x0008 C-string:'FLY'
//             MAN  codep:0x224c wordp:0xcaa2 size:0x0030 C-string:'MAN'
//             .AS  codep:0x224c wordp:0xcada size:0x0006 C-string:'DrawAS'
//          JMPSHP  codep:0x224c wordp:0xcaeb size:0x0006 C-string:'JMPSHP'
//          UNNEST  codep:0x224c wordp:0xcafc size:0x0006 C-string:'UNNEST'
//           >NEST  codep:0x224c wordp:0xcb0c size:0x0006 C-string:'_gt_NEST'
//          GET-HA  codep:0x224c wordp:0xcb1d size:0x0006 C-string:'GET_dash_HA'
//          OV.MVS  codep:0x224c wordp:0xcb2e size:0x0006 C-string:'OV_dot_MVS'
//          S>PORT  codep:0x224c wordp:0xcb3f size:0x0008 C-string:'S_gt_PORT'
//           USE-E  codep:0x224c wordp:0xcb51 size:0x0006 C-string:'USE_dash_E'
//          MUSE-E  codep:0x224c wordp:0xcb62 size:0x0008 C-string:'MUSE_dash_E'
//         ACR-VOC  codep:0x1ab5 wordp:0xcb72 size:0x0014 C-string:'ACR_dash_VOC'
//          ACR-OV  codep:0x83dd wordp:0xcb8e size:0x0002 C-string:'ACR_dash_OV'
//             U-A  codep:0x224c wordp:0xcb98 size:0x0006 C-string:'U_dash_A'
//        PFIL-VOC  codep:0x1ab5 wordp:0xcba7 size:0x0014 C-string:'PFIL_dash_VOC'
//         PFIL-OV  codep:0x83dd wordp:0xcbc4 size:0x0002 C-string:'PFIL_dash_OV'
//            U-PF  codep:0x224c wordp:0xcbcf size:0x0006 C-string:'U_dash_PF'
//     SHIP-GR-VOC  codep:0x1ab5 wordp:0xcbe1 size:0x0014 C-string:'SHIP_dash_GR_dash_VOC'
//      SHIP-GR-OV  codep:0x83dd wordp:0xcc01 size:0x0002 C-string:'SHIP_dash_GR_dash_OV'
//         OV.MASS  codep:0x224c wordp:0xcc0f size:0x0006 C-string:'OV_dot_MASS'
//          OV.ACC  codep:0x224c wordp:0xcc20 size:0x0006 C-string:'OV_dot_ACC'
//         OV.PODS  codep:0x224c wordp:0xcc32 size:0x0006 C-string:'OV_dot_PODS'
//         OV.SHIP  codep:0x224c wordp:0xcc44 size:0x0006 C-string:'OV_dot_SHIP'
//         OVBALAN  codep:0x224c wordp:0xcc56 size:0x0006 C-string:'OVBALAN'
//         OV.CONF  codep:0x224c wordp:0xcc68 size:0x0006 C-string:'OV_dot_CONF'
//         CONFIG-  codep:0x1ab5 wordp:0xcc7a size:0x0014 C-string:'CONFIG_dash_'
//       CONFIG-OV  codep:0x83dd wordp:0xcc9c size:0x0002 C-string:'CONFIG_dash_OV'
//            U-SC  codep:0x224c wordp:0xcca7 size:0x0006 C-string:'U_dash_SC'
//          TD-VOC  codep:0x1ab5 wordp:0xccb8 size:0x0014 C-string:'TD_dash_VOC'
//           TD-OV  codep:0x83dd wordp:0xccd6 size:0x0002 C-string:'TD_dash_OV'
//            U-TD  codep:0x224c wordp:0xcce1 size:0x0006 C-string:'U_dash_TD'
//            TOSS  codep:0x224c wordp:0xccf0 size:0x0006 C-string:'TOSS'
//          KEY-EL  codep:0x224c wordp:0xcd01 size:0x0006 C-string:'KEY_dash_EL'
//          OP-VOC  codep:0x1ab5 wordp:0xcd12 size:0x0014 C-string:'OP_dash_VOC'
//           OP-OV  codep:0x83dd wordp:0xcd30 size:0x0002 C-string:'OP_dash_OV'
//            U-OP  codep:0x224c wordp:0xcd3b size:0x0006 C-string:'U_dash_OP'
//            VITA  codep:0x1ab5 wordp:0xcd4a size:0x0014 C-string:'VITA'
//         VITA-OV  codep:0x83dd wordp:0xcd6a size:0x0002 C-string:'VITA_dash_OV'
//             DIO  codep:0x224c wordp:0xcd74 size:0x0006 C-string:'DIO'
//        MAPS-VOC  codep:0x1ab5 wordp:0xcd87 size:0x0014 C-string:'MAPS_dash_VOC'
//           MAPS-  codep:0x83dd wordp:0xcda5 size:0x0002 C-string:'MAPS_dash_'
//            MAPS  codep:0x224c wordp:0xcdb0 size:0x0006 C-string:'MAPS'
//           .STOR  codep:0x224c wordp:0xcdc0 size:0x0006 C-string:'DrawSTOR'
//           (2X2)  codep:0x224c wordp:0xcdd0 size:0x0006 C-string:'_ro_2X2_rc_'
//           2X2CO  codep:0x224c wordp:0xcde0 size:0x0008 C-string:'_2X2CO'
//           (4X4)  codep:0x224c wordp:0xcdf2 size:0x0006 C-string:'_ro_4X4_rc_'
//           4X4CO  codep:0x224c wordp:0xce02 size:0x0008 C-string:'_4X4CO'
//           (8X8)  codep:0x224c wordp:0xce14 size:0x0006 C-string:'_ro_8X8_rc_'
//           8X8CO  codep:0x224c wordp:0xce24 size:0x0008 C-string:'_8X8CO'
//         STO-VOC  codep:0x1ab5 wordp:0xce34 size:0x0014 C-string:'STO_dash_VOC'
//          STO-OV  codep:0x83dd wordp:0xce50 size:0x0002 C-string:'STO_dash_OV'
//             DO.  codep:0x224c wordp:0xce5a size:0x0006 C-string:'DO_dot_'
//             INJ  codep:0x224c wordp:0xce68 size:0x0006 C-string:'INJ'
//         DRONE-V  codep:0x1ab5 wordp:0xce7a size:0x0014 C-string:'DRONE_dash_V'
//         DRONE-O  codep:0x83dd wordp:0xce9a size:0x0002 C-string:'DRONE_dash_O'
//             DRN  codep:0x224c wordp:0xcea4 size:0x0008 C-string:'DRN'
//           RECAL  codep:0x224c wordp:0xceb6 size:0x0008 C-string:'RECAL'
//           DRONE  codep:0x224c wordp:0xcec8 size:0x0008 C-string:'DRONE'
//         TVCON-V  codep:0x1ab5 wordp:0xcedc size:0x0014 C-string:'TVCON_dash_V'
//        TVCON-OV  codep:0x83dd wordp:0xcefd size:0x0002 C-string:'TVCON_dash_OV'
//            DOST  codep:0x224c wordp:0xcf08 size:0x0006 C-string:'DOST'
//        DO-STORM  codep:0x224c wordp:0xcf1b size:0x0008 C-string:'DO_dash_STORM'
//             .ST  codep:0x224c wordp:0xcf2b size:0x0006 C-string:'DrawST'
//             .WH  codep:0x224c wordp:0xcf39 size:0x0006 C-string:'DrawWH'
//             .DA  codep:0x224c wordp:0xcf47 size:0x0006 C-string:'DrawDA'
//             .EN  codep:0x224c wordp:0xcf55 size:0x0006 C-string:'DrawEN'
//             .CA  codep:0x224c wordp:0xcf63 size:0x0006 C-string:'DrawCA'
//             .DI  codep:0x224c wordp:0xcf71 size:0x0006 C-string:'DrawDI'
//        SEED-VOC  codep:0x1ab5 wordp:0xcf80 size:0x0014 C-string:'SEED_dash_VOC'
//         SEED-OV  codep:0x83dd wordp:0xcf9d size:0x0002 C-string:'SEED_dash_OV'
//            POPU  codep:0x224c wordp:0xcfa8 size:0x0006 C-string:'POPU'
//           ITEMS  codep:0x1ab5 wordp:0xcfb8 size:0x0014 C-string:'ITEMS'
//        ITEMS-OV  codep:0x83dd wordp:0xcfd9 size:0x0002 C-string:'ITEMS_dash_OV'
//          /ITEMS  codep:0x224c wordp:0xcfe6 size:0x0006 C-string:'_slash_ITEMS'
//         >DEBRIS  codep:0x224c wordp:0xcff8 size:0x0006 C-string:'_gt_DEBRIS'
//          ICON-V  codep:0x1ab5 wordp:0xd009 size:0x0014 C-string:'ICON_dash_V'
//          LISTIC  codep:0x83dd wordp:0xd028 size:0x0002 C-string:'LISTIC'
//           ICONS  codep:0x224c wordp:0xd034 size:0x0006 C-string:'ICONS'
//        MOVE-VOC  codep:0x1ab5 wordp:0xd047 size:0x0014 C-string:'MOVE_dash_VOC'
//         MOVE-OV  codep:0x83dd wordp:0xd067 size:0x0002 C-string:'MOVE_dash_OV'
//            TVMO  codep:0x224c wordp:0xd072 size:0x0006 C-string:'TVMO'
//            ?POP  codep:0x224c wordp:0xd081 size:0x0006 C-string:'IsPOP'
//            !E/M  codep:0x224c wordp:0xd090 size:0x0006 C-string:'StoreE_slash_M'
//          TELE_1  codep:0x224c wordp:0xd09f size:0x0006 C-string:'TELE_1'
//          TELE_2  codep:0x224c wordp:0xd0ae size:0x0008 C-string:'TELE_2'
//            T.V.  codep:0x1ab5 wordp:0xd0bf size:0x0014 C-string:'T_dot_V_dot_'
//           TV-OV  codep:0x83dd wordp:0xd0dd size:0x0002 C-string:'TV_dash_OV'
//           TV-DI  codep:0x224c wordp:0xd0e9 size:0x0006 C-string:'TV_dash_DI'
//           (TVSE  codep:0x224c wordp:0xd0f9 size:0x0006 C-string:'_ro_TVSE'
//           (CLEA  codep:0x224c wordp:0xd109 size:0x0006 C-string:'_ro_CLEA'
//           (HEAV  codep:0x224c wordp:0xd119 size:0x0006 C-string:'_ro_HEAV'
//           (WEAP  codep:0x224c wordp:0xd129 size:0x0006 C-string:'_ro_WEAP'
//           (TALK  codep:0x224c wordp:0xd139 size:0x0006 C-string:'_ro_TALK'
//           HEAVE  codep:0x224c wordp:0xd149 size:0x000a C-string:'HEAVE'
//         BEHAVIO  codep:0x1ab5 wordp:0xd15f size:0x0014 C-string:'BEHAVIO'
//           BEHAV  codep:0x83dd wordp:0xd17d size:0x0002 C-string:'BEHAV'
//           TVTAS  codep:0x224c wordp:0xd189 size:0x0006 C-string:'TVTAS'
//              TV  codep:0x224c wordp:0xd196 size:0x0006 C-string:'TV'
//           (SIMU  codep:0x224c wordp:0xd1a6 size:0x0006 C-string:'_ro_SIMU'
//           SIMUL  codep:0x224c wordp:0xd1b6 size:0x0008 C-string:'SIMUL'
//            FSTN  codep:0x224c wordp:0xd1c7 size:0x0006 C-string:'FSTN'
//           DSTUN  codep:0x224c wordp:0xd1d7 size:0x0008 C-string:'DSTUN'
//         STP-VOC  codep:0x1ab5 wordp:0xd1eb size:0x0014 C-string:'STP_dash_VOC'
//          STP-OV  codep:0x83dd wordp:0xd20a size:0x0002 C-string:'STP_dash_OV'
//           SECUR  codep:0x224c wordp:0xd216 size:0x0006 C-string:'SECUR'
//           SIC'E  codep:0x224c wordp:0xd226 size:0x0006 C-string:'SIC_i_E'
//            2NDS  codep:0x224c wordp:0xd235 size:0x0006 C-string:'_2NDS'
//        LAUNCH-V  codep:0x1ab5 wordp:0xd248 size:0x0014 C-string:'LAUNCH_dash_V'
//           LAUNC  codep:0x83dd wordp:0xd266 size:0x0002 C-string:'LAUNC'
//           OVCOU  codep:0x224c wordp:0xd272 size:0x0008 C-string:'OVCOU'
//           OVBAC  codep:0x224c wordp:0xd284 size:0x0008 C-string:'OVBAC'
//           .AIRL  codep:0x224c wordp:0xd296 size:0x0006 C-string:'DrawAIRL'
//         CAP-VOC  codep:0x1ab5 wordp:0xd2a8 size:0x0014 C-string:'CAP_dash_VOC'
//           CAP-O  codep:0x83dd wordp:0xd2c6 size:0x0002 C-string:'CAP_dash_O'
//           OV/IT  codep:0x224c wordp:0xd2d2 size:0x0006 C-string:'OV_slash_IT'
//           OV/LA  codep:0x224c wordp:0xd2e2 size:0x0006 C-string:'OV_slash_LA'
//           OV>DE  codep:0x224c wordp:0xd2f2 size:0x0006 C-string:'OV_gt_DE'
//           OV/LO  codep:0x224c wordp:0xd302 size:0x0006 C-string:'OV_slash_LO'
//         DOC-VOC  codep:0x1ab5 wordp:0xd314 size:0x0014 C-string:'DOC_dash_VOC'
//          DOC-OV  codep:0x83dd wordp:0xd333 size:0x0002 C-string:'DOC_dash_OV'
//           OV/EX  codep:0x224c wordp:0xd33f size:0x0006 C-string:'OV_slash_EX'
//           OV/TR  codep:0x224c wordp:0xd34f size:0x0006 C-string:'OV_slash_TR'
//          AN-VOC  codep:0x1ab5 wordp:0xd360 size:0x0014 C-string:'AN_dash_VOC'
//           AN-OV  codep:0x83dd wordp:0xd37e size:0x0002 C-string:'AN_dash_OV'
//           OV/AN  codep:0x224c wordp:0xd38a size:0x0006 C-string:'OV_slash_AN'
//            *MAP  codep:0x1ab5 wordp:0xd399 size:0x0014 C-string:'_star_MAP'
//         *MAP-OV  codep:0x83dd wordp:0xd3b9 size:0x0002 C-string:'_star_MAP_dash_OV'
//           OV/ST  codep:0x224c wordp:0xd3c5 size:0x000c C-string:'OV_slash_ST'
//         JUMP-VO  codep:0x1ab5 wordp:0xd3dd size:0x0014 C-string:'JUMP_dash_VO'
//         JUMP-OV  codep:0x83dd wordp:0xd3fd size:0x0002 C-string:'JUMP_dash_OV'
//            JUMP  codep:0x224c wordp:0xd408 size:0x0006 C-string:'JUMP'
//         LAND-VO  codep:0x1ab5 wordp:0xd41a size:0x0014 C-string:'LAND_dash_VO'
//           LAND-  codep:0x83dd wordp:0xd438 size:0x0002 C-string:'LAND_dash_'
//         TRY-L_2  codep:0x224c wordp:0xd444 size:0x0006 C-string:'TRY_dash_L_2'
//         TRY-L_1  codep:0x224c wordp:0xd454 size:0x0006 C-string:'TRY_dash_L_1'
//         BTN-VOC  codep:0x1ab5 wordp:0xd466 size:0x0014 C-string:'BTN_dash_VOC'
//          BTN-OV  codep:0x83dd wordp:0xd485 size:0x0002 C-string:'BTN_dash_OV'
//           OVFLT  codep:0x224c wordp:0xd491 size:0x0006 C-string:'OVFLT'
//          CHKFLT  codep:0x1ab5 wordp:0xd4a2 size:0x0014 C-string:'CHKFLT'
//           CHKFL  codep:0x83dd wordp:0xd4c0 size:0x0002 C-string:'CHKFL'
//           ?FLIG  codep:0x224c wordp:0xd4cc size:0x0006 C-string:'IsFLIG'
//          PM-VOC  codep:0x1ab5 wordp:0xd4dd size:0x0014 C-string:'PM_dash_VOC'
//           PM-OV  codep:0x83dd wordp:0xd4fb size:0x0002 C-string:'PM_dash_OV'
//           PORTM  codep:0x224c wordp:0xd507 size:0x0006 C-string:'PORTM'
//            PPIC  codep:0x224c wordp:0xd516 size:0x0006 C-string:'PPIC'
//            GAME  codep:0x1ab5 wordp:0xd525 size:0x0014 C-string:'GAME'
//         GAME-OV  codep:0x83dd wordp:0xd545 size:0x0002 C-string:'GAME_dash_OV'
//         STARTGA  codep:0x224c wordp:0xd553 size:0x0006 C-string:'STARTGA'
//            ERR!  codep:0x224c wordp:0xd562 size:0x0006 C-string:'ERR_ex_'
//            MERR  codep:0x224c wordp:0xd571 size:0x0008 C-string:'MERR'
//         GAMEOPS  codep:0x224c wordp:0xd585 size:0x0008 C-string:'GAMEOPS'
//         GAMEOPM  codep:0x224c wordp:0xd599 size:0x0008 C-string:'GAMEOPM'
//            BOSS  codep:0x224c wordp:0xd5aa size:0x0006 C-string:'BOSS'
//             DBS  codep:0x224c wordp:0xd5b8 size:0x0008 C-string:'DBS'
//         LET-THE  codep:0x224c wordp:0xd5cc size:0x0000 C-string:'LET_dash_THE'

// =================================
// =========== VARIABLES ===========
// =================================
const unsigned short int pp__ro_TIME_rc_ = 0x0188; // (TIME) size: 104
// {0x00, 0x00, 0xbf, 0x35, 0x08, 0x00, 0x2e, 0x81, 0x06, 0x8a, 0x01, 0x37, 0x00, 0x2e, 0x83, 0x16, 0x88, 0x01, 0x00, 0x2e, 0xc7, 0x06, 0x93, 0x01, 0x37, 0x00, 0x2e, 0xff, 0x0e, 0x8c, 0x01, 0x75, 0x0e, 0x2e, 0xc7, 0x06, 0x8c, 0x01, 0x0f, 0x00, 0x2e, 0xc7, 0x06, 0x93, 0x01, 0x36, 0x00, 0xcf, 0x00, 0x00, 0x00, 0x00, 0x80, 0x05, 0x8b, 0x2c, 0x62, 0x66, 0xc4, 0x02, 0x58, 0x40, 0x50, 0x29, 0xc0, 0x29, 0xd2, 0xcf, 0x31, 0xdb, 0xf7, 0xf3, 0xc3, 0x58, 0x8b, 0xc8, 0x81, 0xe8, 0xd0, 0x01, 0x75, 0x06, 0xb8, 0xc7, 0x01, 0xe9, 0x04, 0x00, 0xb8, 0xc4, 0x01, 0x41, 0x8c, 0xda, 0x31, 0xdb, 0x8e, 0xdb, 0x89, 0x07, 0x8e, 0xda, 0x51, 0xcf}

const unsigned short int pp_OPERATOR = 0x078c; // OPERATOR size: 132
// {0xf4, 0xf6, 0xd0, 0xf7, 0xf0, 0xd5, 0xe6, 0x39, 0x00, 0x00, 0x31, 0x00, 0x27, 0xd5, 0x84, 0x00, 0x74, 0x00, 0x0c, 0x00, 0xf4, 0xf6, 0xd4, 0x03, 0x00, 0xb8, 0x18, 0x00, 0x00, 0x00, 0x18, 0x4f, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x03, 0xbc, 0x03, 0x0a, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x81, 0xd7, 0x00, 0x00, 0x82, 0x01, 0x03, 0x00, 0x00, 0x00, 0x1a, 0x08, 0xf4, 0xf6, 0x1a, 0x08, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x00, 0xe1, 0x17, 0xbc, 0x25, 0x45, 0x1c, 0x6b, 0x26, 0xcf, 0x26, 0xee, 0x26, 0xbb, 0x1c, 0x48, 0x36, 0x31, 0x27, 0x3e, 0x1d, 0xd8, 0x1d, 0x5c, 0x1a, 0x23, 0x1e, 0xd7, 0x25, 0xd1, 0x13, 0x4c, 0x27, 0x67, 0x27, 0xd7, 0x36, 0x90, 0x26, 0x68, 0x1e, 0x06, 0x1f, 0xa9, 0x0f, 0x75, 0x95, 0xb1, 0x24, 0xa9, 0x7b, 0x07, 0xc0, 0xab, 0xbc, 0xb1, 0xad, 0xb4, 0x20}

const unsigned short int pp__n_CACHE = 0x09ef; // #CACHE size: 2
// {0x99, 0x00}

const unsigned short int pp__n_BUFFERS = 0x09fe; // #BUFFERS size: 2
// {0x02, 0x00}

const unsigned short int pp__n_SPACE = 0x0a0b; // #SPACE size: 2
// {0x00, 0x01}

const unsigned short int pp__i_RESTORE_plus_ = 0x0a1b; // 'RESTORE+ size: 2
// {0xbb, 0xb2}

const unsigned short int pp__i_SETUP_plus_ = 0x0a29; // 'SETUP+ size: 2
// {0x8b, 0xb2}

const unsigned short int pp_Get_co_0_star_1_sc_ = 0x0a36; // @,0*1; size: 2
// {0x01, 0x00}

const unsigned short int pp__gt_BREAK = 0x0a43; // >BREAK size: 2
// {0x2c, 0x16}

const unsigned short int pp_BOOT_dash_HOOK = 0x0a53; // BOOT-HOOK size: 2
// {0xcc, 0xd5}

const unsigned short int pp_BOOT_dash_LOAD = 0x0a63; // BOOT-LOAD size: 2
// {0x00, 0x00}

const unsigned short int pp_BUFFER_dash_BEGIN = 0x0a76; // BUFFER-BEGIN size: 2
// {0xe0, 0x42}

const unsigned short int pp_BUFFER_dash_HEAD = 0x0a88; // BUFFER-HEAD size: 2
// {0x00, 0x00}

const unsigned short int pp_CACHE_dash_BEGIN = 0x0a9a; // CACHE-BEGIN size: 2
// {0x00, 0x00}

const unsigned short int pp_CACHE_dash_HEAD = 0x0aab; // CACHE-HEAD size: 2
// {0x00, 0x00}

const unsigned short int pp_REGISTERS = 0x16b4; // REGISTERS size: 23
// {0x02, 0x02, 0x00, 0x15, 0x56, 0x48, 0xc6, 0x07, 0xca, 0x2d, 0x38, 0xd3, 0xca, 0x2d, 0xf1, 0xd5, 0x63, 0x33, 0x63, 0x33, 0xcd, 0x21, 0xc3}

const unsigned short int pp_BEEPMS = 0x25fe; // BEEPMS size: 2
// {0xfa, 0x00}

const unsigned short int pp_BEEPTONE = 0x260d; // BEEPTONE size: 2
// {0xf4, 0x01}

const unsigned short int pp__i_SVBUF = 0x2b83; // 'SVBUF size: 2
// {0x50, 0x7b}

const unsigned short int pp__i_MTBUF = 0x2b90; // 'MTBUF size: 2
// {0xd4, 0xb1}

const unsigned short int pp__n_SECS = 0x2b9c; // #SECS size: 2
// {0x02, 0x00}

const unsigned short int pp__i_FCB = 0x2ba7; // 'FCB size: 2
// {0xca, 0x2d}

const unsigned short int pp__i_CACHE = 0x2bb4; // 'CACHE size: 2
// {0x98, 0x00}

const unsigned short int pp__i_DOSMOUNT = 0x2bc4; // 'DOSMOUNT size: 2
// {0xda, 0x46}

const unsigned short int pp__i_DOSUNMOUNT = 0x2bd6; // 'DOSUNMOUNT size: 2
// {0x44, 0x47}

const unsigned short int pp_REUSEC = 0x2be3; // REUSEC size: 2
// {0x98, 0x00}

const unsigned short int pp__1STCACHEBUF = 0x2bf5; // 1STCACHEBUF size: 2
// {0x8b, 0x43}

const unsigned short int pp_BLKCACHE = 0x2c04; // BLKCACHE size: 2
// {0x63, 0x43}

const unsigned short int pp_DEFAULTDRV = 0x2c15; // DEFAULTDRV size: 2
// {0x2c, 0x6f}

const unsigned short int pp_DISK_dash_ERROR = 0x2c26; // DISK-ERROR size: 2
// {0x00, 0x00}

const unsigned short int pp_DRIVE = 0x2c32; // DRIVE size: 2
// {0x04, 0x00}

const unsigned short int pp_DTA_1 = 0x2c3c; // DTA_1 size: 4
// {0x00, 0x01, 0x63, 0x33}

const unsigned short int pp_FILE = 0x2c49; // FILE size: 2
// {0x02, 0x00}

const unsigned short int pp_FRESH = 0x2c55; // FRESH size: 2
// {0x4c, 0xc5}

const unsigned short int pp_HEAD = 0x2c60; // HEAD size: 2
// {0x00, 0x00}

const unsigned short int pp_LPREV = 0x2c6c; // LPREV size: 2
// {0xe0, 0x42}

const unsigned short int pp_OFFSET = 0x2c79; // OFFSET size: 2
// {0x68, 0x01}

const unsigned short int pp_PREV = 0x2c84; // PREV size: 2
// {0xe0, 0x42}

const unsigned short int pp_SEC = 0x2c8e; // SEC size: 2
// {0x05, 0x00}

const unsigned short int pp_SEGCACHE = 0x2c9d; // SEGCACHE size: 2
// {0x77, 0x43}

const unsigned short int pp_SYSK = 0x2ca8; // SYSK size: 2
// {0x80, 0x02}

const unsigned short int pp_TRACK = 0x2cb4; // TRACK size: 2
// {0x00, 0x00}

const unsigned short int pp_USE = 0x2cbe; // USE size: 2
// {0x21, 0x43}

const unsigned short int pp__eq_DRIVESIZES = 0x2cd0; // =DRIVESIZES size: 6
// {0x68, 0x01, 0x68, 0x01, 0x00, 0x50}

const unsigned short int pp__eq_DRIVENUMBERS = 0x2d23; // =DRIVENUMBERS size: 4
// {0x00, 0x00, 0x00, 0x00}

const unsigned short int pp__eq_FCBPFAS = 0x2d34; // =FCBPFAS size: 8
// {0x00, 0x27, 0x00, 0x00, 0x9e, 0x2d, 0xca, 0x2d}

const unsigned short int pp__eq_FILEBLOCKS = 0x2d4c; // =FILEBLOCKS size: 8
// {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}

const unsigned short int pp__eq_FILETYPES = 0x2d63; // =FILETYPES size: 4
// {0xff, 0xff, 0xff, 0xff}

const unsigned short int pp__eq_OFFSETS = 0x2d74; // =OFFSETS size: 8
// {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}

const unsigned short int pp__eq_R_slash_W_i_S = 0x2d87; // =R/W'S size: 8
// {0x31, 0x36, 0x44, 0x33, 0xe3, 0x43, 0xe3, 0x43}

const unsigned short int pp_FCB = 0x2d97; // FCB size: 88
// {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x50, 0x52, 0x4f, 0x54, 0x45, 0x43, 0x54, 0x20, 0x42, 0x4c, 0x4b, 0x00, 0x00, 0x00, 0x04, 0x00, 0x6c, 0x00, 0x00, 0x49, 0x13, 0x02, 0x8e, 0x03, 0x04, 0x42, 0xef, 0x25, 0x0a, 0x66, 0x0b, 0x1a, 0x1a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x53, 0x54, 0x41, 0x52, 0x46, 0x4c, 0x54, 0x32, 0x43, 0x4f, 0x4d, 0x00, 0x00, 0xf0, 0xd4, 0x00, 0x00, 0x00, 0x00, 0xaa, 0x14, 0x85, 0x8a, 0x01, 0x04, 0x42, 0x00, 0x00, 0x07, 0x66, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00}

const unsigned short int pp_FILENAMES = 0x2dfd; // FILENAMES size: 44
// {0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20}

const unsigned short int pp__co_LXCHG_cc_ = 0x2f36; // {LXCHG} size: 16
// {0x51, 0x26, 0x8b, 0x0f, 0x87, 0xd8, 0x26, 0x87, 0x0f, 0x87, 0xd8, 0x26, 0x89, 0x0f, 0x59, 0xc3}

const unsigned short int pp_DIRNAME = 0x3408; // DIRNAME size: 11
// {0x2d, 0x2d, 0x64, 0x69, 0x72, 0x65, 0x63, 0x74, 0x2d, 0x2d, 0x20}

const unsigned short int pp__i_UNRAVEL = 0x3a60; // 'UNRAVEL size: 2
// {0x8a, 0x8e}

const unsigned short int pp_TRACE = 0x3b1b; // TRACE size: 2
// {0x00, 0x00}

const unsigned short int pp__bo_TRACE = 0x3b28; // [TRACE size: 2
// {0x48, 0x3a}

const unsigned short int pp_TRACE_bc_ = 0x3b35; // TRACE] size: 2
// {0x48, 0x3a}

const unsigned short int pp_OVA = 0x3c04; // OVA size: 2
// {0x60, 0xeb}

const unsigned short int pp_T_dash_DP = 0x3c0f; // T-DP size: 6
// {0x3e, 0xd5, 0x27, 0xf4, 0x3e, 0xd5}

const unsigned short int pp__ro_2V_c__rc_ = 0x3e2d; // (2V:) size: 4
// {0x54, 0x20, 0x2d, 0x20}

const unsigned short int pp_MAXDRV = 0x40d4; // MAXDRV size: 2
// {0x05, 0x00}

const unsigned short int pp_ASKMOUN = 0x460e; // ASKMOUN size: 2
// {0x3b, 0xaa}

const unsigned short int pp__i_BYE = 0x4619; // 'BYE size: 2
// {0x5d, 0x08}

const unsigned short int pp_SKIPPED = 0x4627; // SKIPPED size: 2
// {0x00, 0x00}

const unsigned short int pp_CANSKIP = 0x4635; // CANSKIP size: 2
// {0x00, 0x00}

const unsigned short int pp_W484E = 0x484e; // W484E size: 2
// {0x90, 0x8d}

const unsigned short int pp_W4852 = 0x4852; // W4852 size: 2
// {0x4e, 0x95}

const unsigned short int pp_W4856 = 0x4856; // W4856 size: 4
// {0x4e, 0x95, 0x00, 0x00}

const unsigned short int pp_W4901 = 0x4901; // W4901 size: 17
// {0x00, 0x52, 0x32, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x4f, 0x56, 0x4c, 0x00, 0x00, 0x00, 0x00, 0x41}

const unsigned short int pp_ZZZ = 0x49b4; // ZZZ size: 10
// {0x76, 0x02, 0x19, 0x00, 0x4f, 0x54, 0x20, 0x20, 0x6d, 0x65}

const unsigned short int pp_W4AB8 = 0x4ab8; // W4AB8 size: 16
// {0x51, 0x26, 0x8a, 0x0f, 0x87, 0xd8, 0x26, 0x86, 0x0f, 0x87, 0xd8, 0x26, 0x88, 0x0f, 0x59, 0xc3}

const unsigned short int pp_SEED_3 = 0x4b9c; // SEED_3 size: 2
// {0xdf, 0x97}

const unsigned short int pp_SPHEREW = 0x4d39; // SPHEREW size: 2
// {0x50, 0x20}

const unsigned short int pp_SIGNEXT = 0x4d47; // SIGNEXT size: 2
// {0x44, 0x20}

const unsigned short int pp_W4D4B = 0x4d4b; // W4D4B size: 2
// {0x3a, 0x20}

const unsigned short int pp_W4D4F = 0x4d4f; // W4D4F size: 2
// {0x3a, 0x20}

const unsigned short int pp_W4D53 = 0x4d53; // W4D53 size: 2
// {0x3a, 0x20}

const unsigned short int pp__i_ARRAY = 0x4d60; // 'ARRAY size: 2
// {0x82, 0x6a}

const unsigned short int pp_XLL = 0x4eb8; // XLL size: 2
// {0x41, 0x44}

const unsigned short int pp_YLL = 0x4ec2; // YLL size: 2
// {0x72, 0x65}

const unsigned short int pp_XUR = 0x4ecc; // XUR size: 2
// {0x72, 0x65}

const unsigned short int pp_YUR = 0x4ed6; // YUR size: 2
// {0x72, 0x65}

const unsigned short int pp_FONT_n_ = 0x4f54; // FONT# size: 2
// {0x00, 0x00}

const unsigned short int pp_TANDY = 0x4f60; // TANDY size: 2
// {0x10, 0x00}

const unsigned short int pp_COLORT = 0x4f6d; // COLORT size: 17
// {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00}

const unsigned short int pp_IsNID = 0x505f; // ?NID size: 2
// {0x5d, 0x02}

const unsigned short int pp_BMAP = 0x506a; // BMAP size: 14
// {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}

const unsigned short int pp_WIN = 0x5176; // WIN size: 2
// {0x00, 0x00}

const unsigned short int pp_IsUF = 0x51db; // ?UF size: 2
// {0x00, 0x00}

const unsigned short int pp_IsAF = 0x51e5; // ?AF size: 2
// {0x00, 0x00}

const unsigned short int pp__n_SHOTS = 0x51f2; // #SHOTS size: 2
// {0x75, 0x02}

const unsigned short int pp__i_ASYS = 0x51fe; // 'ASYS size: 2
// {0xda, 0xca}

const unsigned short int pp_P_slash_B = 0x5208; // P/B size: 2
// {0x02, 0x00}

const unsigned short int pp__i_BOSS = 0x5214; // 'BOSS size: 2
// {0xb8, 0xd5}

const unsigned short int pp__pe_TALK = 0x5220; // %TALK size: 2
// {0xe8, 0x29}

const unsigned short int pp_CLIP_dash_TA = 0x53d5; // CLIP-TA size: 20
// {0x08, 0x08, 0x01, 0xc7, 0x00, 0x02, 0x02, 0x00, 0x9f, 0x00, 0x04, 0x04, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00}

const unsigned short int pp_FONTSEG = 0x5410; // FONTSEG size: 2
// {0x37, 0x7f}

const unsigned short int pp_REMSEG = 0x541d; // REMSEG size: 2
// {0x3e, 0x95}

const unsigned short int pp_XBUF_dash_SE = 0x542b; // XBUF-SE size: 2
// {0x37, 0x6f}

const unsigned short int pp_DICT = 0x5436; // DICT size: 2
// {0xfa, 0x84}

const unsigned short int pp_IsALL = 0x5441; // ?ALL size: 2
// {0xff, 0x00}

const unsigned short int pp_IsYOK = 0x544c; // ?YOK size: 2
// {0xff, 0x00}

const unsigned short int pp_HUB = 0x5456; // HUB size: 2
// {0x01, 0x00}

const unsigned short int pp_PIC_n_ = 0x5461; // PIC# size: 2
// {0x5d, 0x5e}

const unsigned short int pp__1LOGO = 0x546d; // 1LOGO size: 14
// {0x1f, 0x3f, 0xdf, 0xee, 0x0c, 0x33, 0x0c, 0x03, 0xb7, 0xcf, 0x8f, 0x7f, 0x00, 0xc0}

const unsigned short int pp_W547D = 0x547d; // W547D size: 10
// {0xc3, 0x7e, 0xa5, 0xbd, 0xa5, 0xa5, 0xc3, 0xbd, 0x00, 0x7e}

const unsigned short int pp_W5489 = 0x5489; // W5489 size: 6
// {0x18, 0x7a, 0x86, 0x61, 0x80, 0x17}

const unsigned short int pp_W5491 = 0x5491; // W5491 size: 2
// {0x00, 0xfc}

const unsigned short int pp__i_XCOMM = 0x549e; // 'XCOMM size: 2
// {0x32, 0xc3}

const unsigned short int pp_TCLR = 0x54a9; // TCLR size: 2
// {0x0f, 0x00}

const unsigned short int pp__i_TRAK = 0x54b5; // 'TRAK size: 2
// {0x56, 0xca}

const unsigned short int pp_TRAK_dash_HR = 0x54c3; // TRAK-HR size: 2
// {0x4e, 0x20}

const unsigned short int pp_A_dash_STREN = 0x54d1; // A-STREN size: 2
// {0x48, 0x20}

const unsigned short int pp_DIRBLK = 0x54de; // DIRBLK size: 2
// {0x00, 0x00}

const unsigned short int pp_TIMESTA = 0x54ec; // TIMESTA size: 2
// {0x20, 0x01}

const unsigned short int pp_RELAXTI = 0x54fa; // RELAXTI size: 0
// {0x20}

const unsigned short int pp__rc__c__dash__co_601 = 0x5508; // ):-,601 size: 4
// {0x0a, 0x05, 0x01, 0x00}

const unsigned short int pp_ICONFON = 0x5518; // ICONFON size: 2
// {0x01, 0x00}

const unsigned short int pp_LFRAME = 0x5525; // LFRAME size: 2
// {0x53, 0x51}

const unsigned short int pp_IsAUTO = 0x5531; // ?AUTO size: 2
// {0x5d, 0xdf}

const unsigned short int pp_IsCRITIC = 0x553f; // ?CRITIC size: 2
// {0x20, 0x81}

const unsigned short int pp_P_dash_POSTU = 0x554d; // P-POSTU size: 2
// {0x20, 0x1e}

const unsigned short int pp_ELEM_dash_AM = 0x555b; // ELEM-AM size: 2
// {0x14, 0x00}

const unsigned short int pp__i_INJURE = 0x5569; // 'INJURE size: 2
// {0x68, 0xce}

const unsigned short int pp_P_dash_COLOR = 0x5577; // P-COLOR size: 3
// {0x41, 0x4e, 0x05}

const unsigned short int pp_P_dash_PHRAS = 0x5586; // P-PHRAS size: 3
// {0x31, 0x35, 0x05}

const unsigned short int pp_P_dash_CARP = 0x5594; // P-CARP size: 3
// {0x2e, 0x8a, 0x05}

const unsigned short int pp_O_dash_COLOR = 0x55a3; // O-COLOR size: 3
// {0xeb, 0x0e, 0x05}

const unsigned short int pp_HYDRO = 0x55b0; // HYDRO size: 2
// {0x5d, 0x40}

const unsigned short int pp_ATMO = 0x55bb; // ATMO size: 2
// {0x5d, 0x00}

const unsigned short int pp_LCOLOR = 0x55c8; // LCOLOR size: 2
// {0x8e, 0xde}

const unsigned short int pp_IsTV = 0x55d2; // ?TV size: 2
// {0x01, 0x00}

const unsigned short int pp_OLDHR = 0x55de; // OLDHR size: 2
// {0x36, 0x75}

const unsigned short int pp_AVCNT = 0x55ea; // AVCNT size: 2
// {0x5d, 0x2e}

const unsigned short int pp_W55EE = 0x55ee; // W55EE size: 2
// {0x01, 0x00}

const unsigned short int pp_IsBOMB = 0x55fa; // ?BOMB size: 3
// {0x00, 0x00, 0x00}

const unsigned short int pp_W55FF = 0x55ff; // W55FF size: 2
// {0x00, 0x00}

const unsigned short int pp_W5603 = 0x5603; // W5603 size: 2
// {0x00, 0x00}

const unsigned short int pp_W5607 = 0x5607; // W5607 size: 2
// {0xef, 0xf9}

const unsigned short int pp_FILE_n_ = 0x5613; // FILE# size: 2
// {0x12, 0x00}

const unsigned short int pp_RECORD_n_ = 0x5621; // RECORD# size: 2
// {0x00, 0x00}

const unsigned short int pp_W5625 = 0x5625; // W5625 size: 2
// {0xd0, 0xf7}

const unsigned short int pp_W5629 = 0x5629; // W5629 size: 2
// {0xe0, 0xfb}

const unsigned short int pp_CXSP = 0x5634; // CXSP size: 2
// {0x7e, 0x65}

const unsigned short int pp_W5638 = 0x5638; // W5638 size: 2
// {0xd1, 0x65}

const unsigned short int pp_W563C = 0x563c; // W563C size: 2
// {0x3a, 0x20}

const unsigned short int pp_EDL = 0x5646; // EDL size: 2
// {0x00, 0x74}

const unsigned short int pp_A_dash_POSTU = 0x5654; // A-POSTU size: 2
// {0x20, 0xa4}

const unsigned short int pp_W5658 = 0x5658; // W5658 size: 2
// {0xc8, 0x00}

const unsigned short int pp_P_dash_RACES = 0x5666; // P-RACES size: 2
// {0x20, 0x2d}

const unsigned short int pp_W566A = 0x566a; // W566A size: 2
// {0x8c, 0x93}

const unsigned short int pp_W566E = 0x566e; // W566E size: 2
// {0x6d, 0x93}

const unsigned short int pp_W5672 = 0x5672; // W5672 size: 2
// {0xca, 0x93}

const unsigned short int pp_W5676 = 0x5676; // W5676 size: 2
// {0x08, 0x94}

const unsigned short int pp_W567A = 0x567a; // W567A size: 2
// {0x3a, 0x20}

const unsigned short int pp__i_THROW_dash_ = 0x5688; // 'THROW- size: 2
// {0x6e, 0x3a}

const unsigned short int pp__i_MAP = 0x5693; // 'MAP size: 2
// {0x48, 0x3a}

const unsigned short int pp__i_TRAVER = 0x56a1; // 'TRAVER size: 2
// {0xb3, 0x7a}

const unsigned short int pp__i__ask_EXIT = 0x56ae; // '?EXIT size: 2
// {0xec, 0x7d}

const unsigned short int pp_W56B2 = 0x56b2; // W56B2 size: 2
// {0x3a, 0x20}

const unsigned short int pp__i__dot_FLUX_dash_ = 0x56c0; // '.FLUX- size: 2
// {0xf6, 0xe3}

const unsigned short int pp_IsTD = 0x56ca; // ?TD size: 2
// {0x00, 0x00}

const unsigned short int pp_IsLANDED = 0x56d8; // ?LANDED size: 2
// {0x00, 0x00}

const unsigned short int pp_W56DC = 0x56dc; // W56DC size: 2
// {0xf0, 0x01}

const unsigned short int pp_W56E0 = 0x56e0; // W56E0 size: 2
// {0x00, 0x00}

const unsigned short int pp_W56E4 = 0x56e4; // W56E4 size: 2
// {0x60, 0x13}

const unsigned short int pp_W56E8 = 0x56e8; // W56E8 size: 2
// {0x00, 0x00}

const unsigned short int pp_W56EC = 0x56ec; // W56EC size: 2
// {0x5a, 0x0f}

const unsigned short int pp_W56F0 = 0x56f0; // W56F0 size: 2
// {0x4d, 0x12}

const unsigned short int pp_W56F4 = 0x56f4; // W56F4 size: 2
// {0x00, 0x00}

const unsigned short int pp_W56F8 = 0x56f8; // W56F8 size: 2
// {0x0e, 0x00}

const unsigned short int pp__dash_END = 0x5703; // -END size: 2
// {0x04, 0x00}

const unsigned short int pp_OV_n_ = 0x570d; // OV# size: 2
// {0x00, 0x00}

const unsigned short int pp_W5711 = 0x5711; // W5711 size: 2
// {0x85, 0x6f}

const unsigned short int pp_REPAIRT = 0x571f; // REPAIRT size: 2
// {0xff, 0xff}

const unsigned short int pp_HBUF_dash_SE = 0x572d; // HBUF-SE size: 2
// {0x37, 0x6f}

const unsigned short int pp_DBUF_dash_SE = 0x573b; // DBUF-SE size: 2
// {0x00, 0xa0}

const unsigned short int pp_COLOR = 0x5747; // COLOR size: 2
// {0x0f, 0x00}

const unsigned short int pp_DCOLOR = 0x5754; // DCOLOR size: 2
// {0x0f, 0x00}

const unsigned short int pp_YTAB = 0x575f; // YTAB size: 2
// {0x29, 0x4c}

const unsigned short int pp_Y1 = 0x5768; // Y1 size: 2
// {0x5e, 0x89}

const unsigned short int pp_X1 = 0x5771; // X1 size: 2
// {0x89, 0x3e}

const unsigned short int pp_Y2 = 0x577a; // Y2 size: 2
// {0x5e, 0x5e}

const unsigned short int pp_X2 = 0x5783; // X2 size: 2
// {0xb2, 0x08}

const unsigned short int pp_YTABL = 0x578f; // YTABL size: 2
// {0xe8, 0xc7}

const unsigned short int pp_BUF_dash_SEG = 0x579d; // BUF-SEG size: 2
// {0x00, 0xa0}

const unsigned short int pp_RETURN = 0x57aa; // RETURN size: 2
// {0x1e, 0xb8}

const unsigned short int pp_IsWIN = 0x57b5; // ?WIN size: 2
// {0x00, 0x00}

const unsigned short int pp__n_IN = 0x57bf; // #IN size: 2
// {0x04, 0x00}

const unsigned short int pp__n_OUT = 0x57ca; // #OUT size: 2
// {0x41, 0x42}

const unsigned short int pp_VIN = 0x57d4; // VIN size: 2
// {0x8b, 0x68}

const unsigned short int pp_VOUT = 0x57df; // VOUT size: 2
// {0xcd, 0x68}

const unsigned short int pp_OIN = 0x57e9; // OIN size: 2
// {0x0f, 0x69}

const unsigned short int pp_OOUT = 0x57f4; // OOUT size: 2
// {0x51, 0x69}

const unsigned short int pp_IVPTR = 0x5800; // IVPTR size: 2
// {0x0d, 0x3d}

const unsigned short int pp_OVPTR = 0x580c; // OVPTR size: 2
// {0xeb, 0x06}

const unsigned short int pp_ICPTR = 0x5818; // ICPTR size: 2
// {0x16, 0x80}

const unsigned short int pp_OCPTR = 0x5824; // OCPTR size: 2
// {0xb8, 0xff}

const unsigned short int pp_FLIP = 0x582f; // FLIP size: 2
// {0x83, 0xfb}

const unsigned short int pp_TACCPT = 0x583c; // TACCPT size: 2
// {0x06, 0x0b}

const unsigned short int pp_TRJCT = 0x5848; // TRJCT size: 2
// {0x29, 0x52}

const unsigned short int pp_IsOPEN = 0x5854; // ?OPEN size: 2
// {0x00, 0x00}

const unsigned short int pp_IsEVAL = 0x5860; // ?EVAL size: 2
// {0x00, 0x00}

const unsigned short int pp_SX = 0x5869; // SX size: 2
// {0x3c, 0x00}

const unsigned short int pp_SY = 0x5872; // SY size: 2
// {0xc2, 0x00}

const unsigned short int pp_SO = 0x587b; // SO size: 2
// {0x20, 0x53}

const unsigned short int pp_PX = 0x5884; // PX size: 2
// {0x3c, 0x00}

const unsigned short int pp_PY = 0x588d; // PY size: 2
// {0xc6, 0x00}

const unsigned short int pp_PO = 0x5896; // PO size: 2
// {0x72, 0x66}

const unsigned short int pp_IX = 0x589f; // IX size: 2
// {0x39, 0x29}

const unsigned short int pp_IY = 0x58a8; // IY size: 2
// {0x1e, 0x70}

const unsigned short int pp_IO = 0x58b1; // IO size: 2
// {0xff, 0x74}

const unsigned short int pp_COMPARE = 0x58bf; // COMPARE size: 2
// {0x4f, 0x44}

const unsigned short int pp_X_dash_INTER = 0x58cd; // X-INTER size: 2
// {0x19, 0x80}

const unsigned short int pp_ILEFT = 0x58d9; // ILEFT size: 2
// {0x00, 0x00}

const unsigned short int pp_IRIGHT = 0x58e6; // IRIGHT size: 2
// {0x9f, 0x00}

const unsigned short int pp_IBELOW = 0x58f3; // IBELOW size: 2
// {0x00, 0x00}

const unsigned short int pp_IABOVE = 0x5900; // IABOVE size: 2
// {0xc7, 0x00}

const unsigned short int pp__i_FLY = 0x590b; // 'FLY size: 2
// {0x92, 0xca}

const unsigned short int pp__i_UNNEST = 0x5919; // 'UNNEST size: 2
// {0xfc, 0xca}

const unsigned short int pp_IsNEW = 0x5924; // ?NEW size: 2
// {0x00, 0x00}

const unsigned short int pp_FORCED = 0x5931; // FORCED size: 2
// {0x21, 0x5a}

const unsigned short int pp__n_VESS = 0x593d; // #VESS size: 2
// {0x00, 0x00}

const unsigned short int pp_CTCOLOR = 0x594b; // CTCOLOR size: 2
// {0x5a, 0x5b}

const unsigned short int pp_XSTART = 0x5958; // XSTART size: 2
// {0x60, 0x00}

const unsigned short int pp_XEND = 0x5963; // XEND size: 2
// {0x79, 0x00}

const unsigned short int pp_YLINE = 0x596f; // YLINE size: 2
// {0xc6, 0x00}

const unsigned short int pp_Is3 = 0x5978; // ?3 size: 2
// {0x00, 0x00}

const unsigned short int pp__i__dot_HUFF = 0x5985; // '.HUFF size: 2
// {0x45, 0xbe}

const unsigned short int pp_SCAN = 0x5990; // SCAN size: 2
// {0xf9, 0x66}

const unsigned short int pp_LMNT = 0x599b; // LMNT size: 2
// {0x00, 0x00}

const unsigned short int pp_SCAN_plus_ = 0x59a7; // SCAN+ size: 2
// {0xf9, 0x66}

const unsigned short int pp_YMIN = 0x59b2; // YMIN size: 2
// {0xc2, 0x00}

const unsigned short int pp_YMAX = 0x59bd; // YMAX size: 2
// {0xc6, 0x00}

const unsigned short int pp__n_HORIZ = 0x59ca; // #HORIZ size: 2
// {0x02, 0x00}

const unsigned short int pp_MOVED = 0x59d6; // MOVED size: 2
// {0x69, 0x7a}

const unsigned short int pp_MEMSEG = 0x59e3; // MEMSEG size: 2
// {0x19, 0x42}

const unsigned short int pp_MEMOFF = 0x59f0; // MEMOFF size: 2
// {0x00, 0x0a}

const unsigned short int pp_MONITOR = 0x59fe; // MONITOR size: 2
// {0x06, 0x00}

const unsigned short int pp_LOCRADI = 0x5a0c; // LOCRADI size: 2
// {0x4b, 0x00}

const unsigned short int pp__ro_ELIST_rc_ = 0x5a1a; // (ELIST) size: 3
// {0xe8, 0x10, 0x00}

const unsigned short int pp_ICON_h_ = 0x5a27; // ICON^ size: 2
// {0x78, 0x69}

const unsigned short int pp_PAL_h_ = 0x5a32; // PAL^ size: 2
// {0x41, 0x6a}

const unsigned short int pp_YBLT = 0x5a3d; // YBLT size: 2
// {0xc6, 0x00}

const unsigned short int pp_XBLT = 0x5a48; // XBLT size: 2
// {0x40, 0x00}

const unsigned short int pp_XORMODE = 0x5a56; // XORMODE size: 2
// {0x00, 0x00}

const unsigned short int pp_LBLT = 0x5a61; // LBLT size: 2
// {0x08, 0x00}

const unsigned short int pp_WBLT = 0x5a6c; // WBLT size: 2
// {0x08, 0x00}

const unsigned short int pp_ABLT = 0x5a77; // ABLT size: 2
// {0x40, 0x02}

const unsigned short int pp_BLTSEG = 0x5a84; // BLTSEG size: 2
// {0xf2, 0x9d}

const unsigned short int pp_BLT_gt_ = 0x5a8f; // BLT> size: 2
// {0x42, 0x02}

const unsigned short int pp_TILE_dash_PT = 0x5a9d; // TILE-PT size: 2
// {0x9a, 0x69}

const unsigned short int pp_IsFUEL_dash_D = 0x5aab; // ?FUEL-D size: 2
// {0x00, 0x00}

const unsigned short int pp_IsG_dash_AWAR = 0x5ab9; // ?G-AWAR size: 2
// {0x00, 0x00}

const unsigned short int pp_GWF = 0x5ac3; // GWF size: 2
// {0x00, 0x00}

const unsigned short int pp_DXVIS = 0x5acf; // DXVIS size: 2
// {0x41, 0x53}

const unsigned short int pp_DYVIS = 0x5adb; // DYVIS size: 2
// {0x45, 0x20}

const unsigned short int pp_XCON = 0x5ae6; // XCON size: 2
// {0x56, 0x20}

const unsigned short int pp_YCON = 0x5af1; // YCON size: 2
// {0x65, 0x72}

const unsigned short int pp_DXCON = 0x5afd; // DXCON size: 2
// {0x72, 0x20}

const unsigned short int pp_DYCON = 0x5b09; // DYCON size: 2
// {0x6e, 0x20}

const unsigned short int pp_XVIS = 0x5b14; // XVIS size: 2
// {0x69, 0x6e}

const unsigned short int pp_YVIS = 0x5b1f; // YVIS size: 2
// {0x65, 0x72}

const unsigned short int pp_XLLDEST = 0x5b2d; // XLLDEST size: 2
// {0x00, 0x00}

const unsigned short int pp_YLLDEST = 0x5b3b; // YLLDEST size: 2
// {0x00, 0x00}

const unsigned short int pp_GLOBALS = 0x5b49; // GLOBALS size: 2
// {0x44, 0x20}

const unsigned short int pp__i__dot_CELL = 0x5b56; // '.CELL size: 2
// {0x72, 0x72}

const unsigned short int pp__i__dot_BACKG = 0x5b64; // '.BACKG size: 2
// {0x55, 0x4e}

const unsigned short int pp__i_ICON_dash_P = 0x5b72; // 'ICON-P size: 2
// {0x55, 0xba}

const unsigned short int pp__i_ICONBO = 0x5b80; // 'ICONBO size: 2
// {0x65, 0xba}

const unsigned short int pp__i_CC = 0x5b8a; // 'CC size: 2
// {0x3c, 0xc8}

const unsigned short int pp_W5B8E = 0x5b8e; // W5B8E size: 2
// {0x13, 0x93}

const unsigned short int pp_W5B92 = 0x5b92; // W5B92 size: 2
// {0xb9, 0x92}

const unsigned short int pp_W5B96 = 0x5b96; // W5B96 size: 2
// {0x8c, 0x92}

const unsigned short int pp_W5B9A = 0x5b9a; // W5B9A size: 2
// {0x5f, 0x92}

const unsigned short int pp_W5B9E = 0x5b9e; // W5B9E size: 2
// {0x05, 0x92}

const unsigned short int pp_IHSEG = 0x5baa; // IHSEG size: 2
// {0xd8, 0x91}

const unsigned short int pp_IGLOBAL = 0x5bb8; // IGLOBAL size: 2
// {0x00, 0x00}

const unsigned short int pp_ILOCAL = 0x5bc5; // ILOCAL size: 2
// {0x00, 0x00}

const unsigned short int pp_IINDEX = 0x5bd2; // IINDEX size: 2
// {0x00, 0x00}

const unsigned short int pp_XWLL = 0x5bdd; // XWLL size: 2
// {0x64, 0x65}

const unsigned short int pp_YWLL = 0x5be8; // YWLL size: 0
// {0x20}

const unsigned short int pp_XWUR = 0x5bf3; // XWUR size: 2
// {0x72, 0x64}

const unsigned short int pp_YWUR = 0x5bfe; // YWUR size: 2
// {0x20, 0x77}

const unsigned short int pp__star_GLOBAL = 0x5c0c; // *GLOBAL size: 2
// {0xa9, 0x01}

const unsigned short int pp__ro_STOP_dash_C = 0x5c1a; // (STOP-C size: 2
// {0x4d, 0x29}

const unsigned short int pp_W5C1E = 0x5c1e; // W5C1E size: 2
// {0x00, 0x00}

const unsigned short int pp_CONTEXT_3 = 0x5c2c; // CONTEXT_3 size: 2
// {0x03, 0x00}

const unsigned short int pp__pe_EFF = 0x5c37; // %EFF size: 2
// {0x64, 0x00}

const unsigned short int pp_STORM = 0x5c43; // STORM size: 2
// {0x00, 0x00}

const unsigned short int pp__i_TVT = 0x5c4e; // 'TVT size: 2
// {0x89, 0xd1}

const unsigned short int pp__i_STORM = 0x5c5b; // 'STORM size: 2
// {0x5a, 0xce}

const unsigned short int pp_E_slash_M = 0x5c65; // E/M size: 2
// {0x66, 0x6f}

const unsigned short int pp_FORCEPT = 0x5c73; // FORCEPT size: 2
// {0x00, 0x00}

const unsigned short int pp__n_STORM = 0x5c80; // #STORM size: 2
// {0x00, 0x00}

const unsigned short int pp_W5C84 = 0x5c84; // W5C84 size: 2
// {0x4a, 0x00}

const unsigned short int pp_W5C88 = 0x5c88; // W5C88 size: 2
// {0x00, 0x00}

const unsigned short int pp_W5C8C = 0x5c8c; // W5C8C size: 2
// {0x4b, 0x00}

const unsigned short int pp_W5C90 = 0x5c90; // W5C90 size: 2
// {0x00, 0x00}

const unsigned short int pp_PORTDAT = 0x5c9e; // PORTDAT size: 2
// {0x00, 0x00}

const unsigned short int pp_IsPORT = 0x5caa; // ?PORT size: 2
// {0x01, 0x00}

const unsigned short int pp_TVIS = 0x5cb5; // TVIS size: 2
// {0x40, 0x00}

const unsigned short int pp_RVIS = 0x5cc0; // RVIS size: 2
// {0x88, 0x00}

const unsigned short int pp_BVIS = 0x5ccb; // BVIS size: 2
// {0xf7, 0xff}

const unsigned short int pp_LVIS = 0x5cd6; // LVIS size: 2
// {0x42, 0x00}

const unsigned short int pp_LFSEG = 0x5ce2; // LFSEG size: 2
// {0x3e, 0x95}

const unsigned short int pp_LSYSEG = 0x5cef; // LSYSEG size: 2
// {0x51, 0x91}

const unsigned short int pp_MSYSEG = 0x5cfc; // MSYSEG size: 2
// {0xd7, 0x90}

const unsigned short int pp_SSYSEG = 0x5d09; // SSYSEG size: 2
// {0x5d, 0x90}

const unsigned short int pp_IsREPAIR = 0x5d17; // ?REPAIR size: 2
// {0x00, 0x00}

const unsigned short int pp_IsHEAL = 0x5d23; // ?HEAL size: 2
// {0x00, 0x00}

const unsigned short int pp_MXNEB = 0x5d2f; // MXNEB size: 2
// {0x41, 0x53}

const unsigned short int pp_THIS_dash_BU = 0x5d3d; // THIS-BU size: 2
// {0x00, 0x00}

const unsigned short int pp_NCRS = 0x5d48; // NCRS size: 2
// {0x20, 0x62}

const unsigned short int pp_OCRS = 0x5d53; // OCRS size: 2
// {0x73, 0x6f}

const unsigned short int pp_WTOP = 0x5d5e; // WTOP size: 2
// {0x33, 0x00}

const unsigned short int pp_WBOTTOM = 0x5d6c; // WBOTTOM size: 2
// {0x01, 0x00}

const unsigned short int pp_WRIGHT = 0x5d79; // WRIGHT size: 2
// {0x9e, 0x00}

const unsigned short int pp_WLEFT = 0x5d85; // WLEFT size: 2
// {0x03, 0x00}

const unsigned short int pp_WLINES = 0x5d92; // WLINES size: 2
// {0x07, 0x00}

const unsigned short int pp_WCHARS = 0x5d9f; // WCHARS size: 2
// {0x26, 0x00}

const unsigned short int pp_SKIP2NE = 0x5dad; // SKIP2NE size: 2
// {0x00, 0x00}

const unsigned short int pp__dash_AIN = 0x5db8; // -AIN size: 2
// {0x74, 0x20}

const unsigned short int pp__i_LAUNCH = 0x5dc6; // 'LAUNCH size: 2
// {0x54, 0xd4}

const unsigned short int pp_IsON_dash_PLA = 0x5dd4; // ?ON-PLA size: 2
// {0x00, 0x00}

const unsigned short int pp_IsRECALL = 0x5de2; // ?RECALL size: 2
// {0x00, 0x00}

const unsigned short int pp_WMSG = 0x5ded; // WMSG size: 2
// {0x00, 0x00}

const unsigned short int pp_CTX = 0x5df7; // CTX size: 2
// {0x00, 0x00}

const unsigned short int pp_CTY = 0x5e01; // CTY size: 2
// {0x00, 0x00}

const unsigned short int pp_FTRIG = 0x5e0d; // FTRIG size: 2
// {0x00, 0x00}

const unsigned short int pp_FQUIT = 0x5e19; // FQUIT size: 2
// {0x74, 0x61}

const unsigned short int pp_LKEY = 0x5e24; // LKEY size: 2
// {0x73, 0x74}

const unsigned short int pp__i_BUTTON = 0x5e32; // 'BUTTON size: 2
// {0x65, 0x20}

const unsigned short int pp_BTN_dash_REC = 0x5e40; // BTN-REC size: 2
// {0x63, 0x74}

const unsigned short int pp_CRSCOLO = 0x5e4e; // CRSCOLO size: 2
// {0x6c, 0x65}

const unsigned short int pp_W5E52 = 0x5e52; // W5E52 size: 2
// {0x3a, 0x20}

const unsigned short int pp_Is_gt_OP = 0x5e5d; // ?>OP size: 2
// {0x00, 0x00}

const unsigned short int pp__i_YANK = 0x5e69; // 'YANK size: 2
// {0x3f, 0xcb}

const unsigned short int pp_Is12 = 0x5e73; // ?12 size: 2
// {0x76, 0x61}

const unsigned short int pp__i__plus_VESS = 0x5e80; // '+VESS size: 2
// {0x44, 0xca}

const unsigned short int pp_IsNEB = 0x5e8b; // ?NEB size: 2
// {0x00, 0x00}

const unsigned short int pp_FORCEKE = 0x5e99; // FORCEKE size: 2
// {0x00, 0x00}

const unsigned short int pp__pe_VAL = 0x5ea4; // %VAL size: 2
// {0x64, 0x00}

const unsigned short int pp_SCROLL_dash_ = 0x5eb2; // SCROLL- size: 2
// {0x4e, 0x20}

const unsigned short int pp__bo__n_CACHE = 0x5ec0; // [#CACHE size: 2
// {0x99, 0x00}

const unsigned short int pp_ESC_dash_EN = 0x5ecd; // ESC-EN size: 2
// {0x00, 0x00}

const unsigned short int pp_ESC_dash_PFA = 0x5edb; // ESC-PFA size: 2
// {0x99, 0xd5}

const unsigned short int pp_LINE_dash_CO = 0x5ee9; // LINE-CO size: 2
// {0x54, 0x20}

const unsigned short int pp_PM_dash_PTR = 0x5ef6; // PM-PTR size: 2
// {0x6f, 0x75}

const unsigned short int pp_W5EFA = 0x5efa; // W5EFA size: 2
// {0x01, 0x00}

const unsigned short int pp_SKEY = 0x5f05; // SKEY size: 2
// {0x63, 0x72}

const unsigned short int pp__n_AUX = 0x5f10; // #AUX size: 2
// {0x05, 0x00}

const unsigned short int pp_IsEGA = 0x5f1b; // ?EGA size: 2
// {0x00, 0x00}

const unsigned short int pp_Is5_c__ro_ = 0x5f26; // ?5:( size: 2
// {0x00, 0x00}

const unsigned short int pp_XABS = 0x5f31; // XABS size: 2
// {0x4a, 0x00}

const unsigned short int pp_YABS = 0x5f3c; // YABS size: 2
// {0x00, 0x00}

const unsigned short int pp_HEADING = 0x5f4a; // HEADING size: 2
// {0x04, 0x00}

const unsigned short int pp__3DSEG = 0x5f56; // 3DSEG size: 2
// {0x69, 0x70}

const unsigned short int pp_VIN_i_ = 0x5f61; // VIN' size: 2
// {0x67, 0x20}

const unsigned short int pp_YSCREEN = 0x5f6f; // YSCREEN size: 0
// {0x20}

const unsigned short int pp_XSCREEN = 0x5f7d; // XSCREEN size: 2
// {0x6f, 0x72}

const unsigned short int pp__i_COMBAT = 0x5f8b; // 'COMBAT size: 2
// {0x7b, 0xc8}

const unsigned short int pp__i_CEX_plus_ = 0x5f97; // 'CEX+ size: 2
// {0xa1, 0xbf}

const unsigned short int pp__i_CEX = 0x5fa2; // 'CEX size: 2
// {0xb5, 0xbf}

const unsigned short int pp__i_WAX = 0x5fad; // 'WAX size: 2
// {0x6b, 0xc8}

const unsigned short int pp_TERMINA = 0x5fbb; // TERMINA size: 2
// {0x44, 0x20}

const unsigned short int pp_IsCOMBAT = 0x5fc9; // ?COMBAT size: 2
// {0x00, 0x00}

const unsigned short int pp_IsATTACK = 0x5fd7; // ?ATTACK size: 2
// {0x74, 0x6f}

const unsigned short int pp_TBOX = 0x5fe2; // TBOX size: 2
// {0x65, 0x61}

const unsigned short int pp_W5FE6 = 0x5fe6; // W5FE6 size: 2
// {0x00, 0x00}

const unsigned short int pp_STAR_dash_HR = 0x5ff4; // STAR-HR size: 2
// {0x00, 0x00}

const unsigned short int pp_STARDAT = 0x6002; // STARDAT size: 2
// {0x00, 0x00}

const unsigned short int pp_TIME_dash_PA = 0x6010; // TIME-PA size: 2
// {0x49, 0x4e}

const unsigned short int pp__n_CLRMAP = 0x601e; // #CLRMAP size: 2
// {0x01, 0x00}

const unsigned short int pp_PLHI = 0x6029; // PLHI size: 2
// {0xff, 0xff}

const unsigned short int pp__i_PROCES = 0x6037; // 'PROCES size: 2
// {0x75, 0x72}

const unsigned short int pp_CURSEG = 0x6044; // CURSEG size: 2
// {0xcb, 0x91}

const unsigned short int pp__i_SIMULA = 0x6052; // 'SIMULA size: 2
// {0xb6, 0xd1}

const unsigned short int pp__i_ENDING = 0x6060; // 'ENDING size: 2
// {0xe0, 0xc8}

const unsigned short int pp__bo_KEYINT = 0x606e; // [KEYINT size: 2
// {0x00, 0x00}

const unsigned short int pp_SIL_h_ = 0x6079; // SIL^ size: 2
// {0x69, 0x6a}

const unsigned short int pp_PIC_h_ = 0x6084; // PIC^ size: 2
// {0x37, 0x6b}

const unsigned short int pp__i_CLEANU = 0x6092; // 'CLEANU size: 2
// {0x20, 0x2d}

const unsigned short int pp__i_KEY_dash_CA = 0x60a0; // 'KEY-CA size: 2
// {0x20, 0x6d}

const unsigned short int pp__i__dot_VITAL = 0x60ae; // '.VITAL size: 2
// {0x49, 0x47}

const unsigned short int pp__i__dot_DATE = 0x60bb; // '.DATE size: 2
// {0x69, 0x74}

const unsigned short int pp__i__dot_VEHIC = 0x60c9; // '.VEHIC size: 2
// {0x2d, 0x53}

const unsigned short int pp__i_VEHICL = 0x60d7; // 'VEHICL size: 2
// {0x43, 0x59}

const unsigned short int pp__i_CREW_dash_C = 0x60e5; // 'CREW-C size: 2
// {0x4c, 0x45}

const unsigned short int pp__i_EXTERN = 0x60f3; // 'EXTERN size: 2
// {0x56, 0xca}

const unsigned short int pp__i_REPAIR = 0x6101; // 'REPAIR size: 2
// {0x74, 0x65}

const unsigned short int pp__i_TREATM = 0x610f; // 'TREATM size: 2
// {0x54, 0x20}

const unsigned short int pp_WEAPON_dash_ = 0x611d; // WEAPON- size: 2
// {0x52, 0x45}

const unsigned short int pp__h_CRIT = 0x6129; // ^CRIT size: 2
// {0x45, 0x20}

const unsigned short int pp_IsFLAT = 0x6135; // ?FLAT size: 2
// {0x74, 0x6f}

const unsigned short int pp__1ST = 0x613f; // 1ST size: 2
// {0x66, 0x20}

const unsigned short int pp_IsMVT = 0x614a; // ?MVT size: 2
// {0x00, 0x00}

const unsigned short int pp_IsSUP = 0x6155; // ?SUP size: 2
// {0x00, 0x00}

const unsigned short int pp_E_dash_USE = 0x6161; // E-USE size: 2
// {0x4c, 0x45}

const unsigned short int pp__i_ENERGY = 0x616f; // 'ENERGY size: 2
// {0x62, 0xcb}

const unsigned short int pp_IsSECURE = 0x617d; // ?SECURE size: 2
// {0x00, 0x00}

const unsigned short int pp__i_STP = 0x6188; // 'STP size: 2
// {0x26, 0xd2}

const unsigned short int pp__i_RSIDE = 0x6195; // 'RSIDE size: 2
// {0x35, 0xd2}

const unsigned short int pp_DERROR = 0x61a2; // DERROR size: 2
// {0x65, 0x20}

const unsigned short int pp_NLR = 0x61ac; // NLR size: 2
// {0x00, 0x00}

const unsigned short int pp_Is10 = 0x61d4; // ?10 size: 2
// {0x00, 0x00}

const unsigned short int pp_BITS = 0x61df; // BITS size: 2
// {0x50, 0x03}

const unsigned short int pp__n_BITS = 0x61eb; // #BITS size: 2
// {0x10, 0x00}

const unsigned short int pp_Is5 = 0x61f4; // ?5 size: 2
// {0x00, 0x00}

const unsigned short int pp_SELLING = 0x6202; // SELLING size: 2
// {0x20, 0x2d}

const unsigned short int pp_BARTERI = 0x6210; // BARTERI size: 2
// {0x20, 0x49}

const unsigned short int pp_IsREPLY = 0x621d; // ?REPLY size: 2
// {0x52, 0x41}

const unsigned short int pp_PLAST = 0x6229; // PLAST size: 2
// {0x20, 0x72}

const unsigned short int pp_PAST = 0x6234; // PAST size: 2
// {0x00, 0x00}

const unsigned short int pp_HAZE = 0x623f; // HAZE size: 4
// {0x00, 0x00, 0x00, 0x00}

const unsigned short int pp_IsCALLIN = 0x624f; // ?CALLIN size: 2
// {0x01, 0x00}

const unsigned short int pp_STAGES = 0x625c; // STAGES size: 14
// {0x63, 0x65, 0x05, 0x41, 0x4c, 0x4c, 0x4f, 0x54, 0x20, 0x65, 0x64, 0x20, 0x0d, 0x0a}

const unsigned short int pp__pe_SLUG = 0x6274; // %SLUG size: 2
// {0x64, 0x00}

const unsigned short int pp_ITEM = 0x627f; // ITEM size: 4
// {0xeb, 0x63, 0x02, 0x6c}

const unsigned short int pp_FSTUN = 0x628d; // FSTUN size: 2
// {0x00, 0x00}

const unsigned short int pp_PATIENC = 0x629b; // PATIENC size: 2
// {0x64, 0x20}

const unsigned short int pp_TMAP = 0x62a6; // TMAP size: 4
// {0x6f, 0x72, 0x20, 0x66}

const unsigned short int pp__c__ro_ = 0x62b1; // :( size: 2
// {0x42, 0xc3}

const unsigned short int pp_TIRED_dash_T = 0x62bf; // TIRED-T size: 4
// {0x45, 0x20, 0x28, 0x73}

const unsigned short int pp_LASTREP = 0x62cf; // LASTREP size: 4
// {0x00, 0x00, 0x00, 0x00}

const unsigned short int pp_TALKCOU = 0x62df; // TALKCOU size: 4
// {0x20, 0x69, 0x74, 0x69}

const unsigned short int pp_VSTIME = 0x62ee; // VSTIME size: 4
// {0x00, 0x00, 0x00, 0x00}

const unsigned short int pp__10_star_CARG = 0x62fe; // 10*CARG size: 4
// {0x00, 0x00, 0xb0, 0x36}

const unsigned short int pp_SENSE_dash_A = 0x630e; // SENSE-A size: 4
// {0x00, 0x00, 0x00, 0x29}

const unsigned short int pp_EYEXY = 0x631c; // EYEXY size: 4
// {0x52, 0x20, 0x64, 0x64}

const unsigned short int pp_WEAPXY = 0x632b; // WEAPXY size: 4
// {0x74, 0x20, 0x53, 0x45}

const unsigned short int pp__10_star_END = 0x633a; // 10*END size: 4
// {0x00, 0x00, 0x98, 0x3a}

const unsigned short int pp_TOWFINE = 0x634a; // TOWFINE size: 4
// {0x16, 0x41, 0x63, 0x74}

const unsigned short int pp_ENC_dash_TIM = 0x635a; // ENC-TIM size: 4
// {0x12, 0x00, 0x23, 0x17}

const unsigned short int pp_NAV_dash_TIM = 0x636a; // NAV-TIM size: 4
// {0x6e, 0x6f, 0x74, 0x20}

const unsigned short int pp_W6370 = 0x6370; // W6370 size: 4
// {0xbf, 0x05, 0xe3, 0x31}

const unsigned short int pp_STIME = 0x637e; // STIME size: 4
// {0x65, 0x6e, 0x74, 0x20}

const unsigned short int pp_ETIME = 0x638c; // ETIME size: 2
// {0x00, 0x00}

const unsigned short int pp_W6390 = 0x6390; // W6390 size: 4
// {0x03, 0x00, 0x40, 0xc6}

const unsigned short int pp_W6396 = 0x6396; // W6396 size: 4
// {0x03, 0x00, 0x40, 0xc6}

const unsigned short int pp_W639C = 0x639c; // W639C size: 4
// {0x00, 0x00, 0x00, 0x00}

const unsigned short int pp_W63A2 = 0x63a2; // W63A2 size: 4
// {0xe5, 0xbf, 0x01, 0x72}

const unsigned short int pp_KEYTIME = 0x63b2; // KEYTIME size: 4
// {0x74, 0x20, 0x6f, 0x66}

const unsigned short int pp_LKEYTIM = 0x63c2; // LKEYTIM size: 4
// {0x69, 0x63, 0x68, 0x20}

const unsigned short int pp__ro_SCROLL_1 = 0x63d2; // (SCROLL_1 size: 4
// {0x4f, 0x58, 0x29, 0x20}

const unsigned short int pp__ro_ORIGIN = 0x63e2; // (ORIGIN size: 4
// {0x4f, 0x52, 0x29, 0x20}

const unsigned short int pp__ro_SCROLL_2 = 0x63f2; // (SCROLL_2 size: 4
// {0x4f, 0x4e, 0x54, 0x29}

const unsigned short int pp_REAL_dash_MS = 0x6402; // REAL-MS size: 4
// {0x00, 0x00, 0x10, 0x27}

const unsigned short int pp_LAST_dash_UP = 0x6412; // LAST-UP size: 4
// {0x00, 0x00, 0x00, 0x00}

const unsigned short int pp_XWLD_c_XP = 0x6422; // XWLD:XP size: 4
// {0x20, 0x00, 0x08, 0x00}

const unsigned short int pp_YWLD_c_YP = 0x6432; // YWLD:YP size: 4
// {0x30, 0x00, 0x08, 0x00}

const unsigned short int pp_ANCHOR = 0x6441; // ANCHOR size: 4
// {0x00, 0x00, 0x4a, 0x00}

const unsigned short int pp_OK_dash_TALK = 0x6451; // OK-TALK size: 4
// {0x49, 0x4d, 0x45, 0x20}

const unsigned short int pp_TVEHICL = 0x6461; // TVEHICL size: 4
// {0xd4, 0x2b, 0x02, 0x56}

const unsigned short int pp_TV_dash_HOLD = 0x6471; // TV-HOLD size: 4
// {0x00, 0x2c, 0x02, 0x61}

const unsigned short int pp_SUPER_dash_B = 0x6481; // SUPER-B size: 4
// {0xaf, 0x10, 0x00, 0x61}

const unsigned short int pp__ro_SYSTEM = 0x6491; // (SYSTEM size: 4
// {0xd1, 0xbf, 0x01, 0x65}

const unsigned short int pp__ro_ORBIT_rc_ = 0x64a1; // (ORBIT) size: 4
// {0xd1, 0x67, 0x02, 0x65}

const unsigned short int pp__ro_PLANET = 0x64b1; // (PLANET size: 4
// {0xe5, 0xbf, 0x01, 0x65}

const unsigned short int pp__ro_SURFAC = 0x64c1; // (SURFAC size: 4
// {0x9c, 0x10, 0x00, 0x65}

const unsigned short int pp__ro_ENCOUN = 0x64d1; // (ENCOUN size: 4
// {0x00, 0x00, 0x00, 0x00}

const unsigned short int pp__ro_SHIPBO = 0x64e1; // (SHIPBO size: 4
// {0x48, 0x2b, 0x02, 0x65}

const unsigned short int pp__ro_AORIGI = 0x64f1; // (AORIGI size: 4
// {0x00, 0x00, 0x00, 0x00}

const unsigned short int pp_THIS_dash_RE = 0x6501; // THIS-RE size: 4
// {0x4f, 0x4e, 0x20, 0x6e}

const unsigned short int pp__ro_THIS_dash_I = 0x6511; // (THIS-I size: 4
// {0x4d, 0x29, 0x20, 0x68}

const unsigned short int pp_W6517 = 0x6517; // W6517 size: 2
// {0x45, 0xb2}

const unsigned short int pp_W651B = 0x651b; // W651B size: 186
// {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb2, 0x17, 0x00, 0x32, 0x7d, 0x02, 0xb2, 0x17, 0x00, 0xba, 0x6c, 0x02, 0x48, 0x2b, 0x02, 0xd1, 0x67, 0x02, 0x48, 0x2b, 0x02, 0xd1, 0x67, 0x02, 0xd1, 0xbf, 0x01, 0x0e, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x29, 0x1d, 0x3a, 0x20, 0x01, 0x00, 0x96, 0xc9, 0x01, 0x00, 0xc3, 0xc9, 0x01, 0x00, 0xa5, 0xc9, 0x01, 0x00, 0xe1, 0xc9, 0x01, 0x00, 0x2c, 0xca, 0x01, 0x00, 0x77, 0xca, 0x01, 0x00, 0xb3, 0xca, 0x01, 0x00, 0xe0, 0xca, 0x01, 0x00, 0x0d, 0xcb, 0x01, 0x00, 0x67, 0xcb, 0x01, 0x00, 0x01, 0x00, 0xb3, 0x7a, 0x00, 0x00, 0x48, 0x3a, 0x09, 0x76, 0x01, 0x00, 0xb3, 0x7a, 0xec, 0x7d, 0x00, 0x00, 0x6e, 0x00}

const unsigned short int pp_IBFR = 0x65de; // IBFR size: 273
// {0x0e, 0x00, 0x00, 0x00, 0x2b, 0x02, 0x25, 0x2b, 0x02, 0x7c, 0x2b, 0x02, 0x12, 0x00, 0x08, 0x00, 0x7b, 0xff, 0xa8, 0xfe, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x0b, 0x41, 0x2b, 0xaa, 0xd8, 0x07, 0xac, 0xeb, 0xab, 0x5a, 0xb4, 0xac, 0x36, 0x8f, 0x1f, 0x61, 0x14, 0x20, 0x57, 0x49, 0x4c, 0x4c, 0x20, 0x42, 0x45, 0x20, 0x4d, 0x45, 0x54, 0x20, 0x57, 0x49, 0x54, 0x48, 0x20, 0x46, 0x4f, 0x52, 0x43, 0x45, 0x2e, 0x20, 0x44, 0x4f, 0x20, 0x59, 0x4f, 0x55, 0x20, 0x41, 0x47, 0x52, 0x45, 0x45, 0x20, 0x54, 0x4f, 0x20, 0x43, 0x4f, 0x4d, 0x45, 0x20, 0x41, 0x4c, 0x4f, 0x4e, 0x47, 0x20, 0x50, 0x45, 0x41, 0x43, 0x45, 0x46, 0x55, 0x4c, 0x4c, 0x59, 0x3f, 0x20, 0x20, 0x25, 0x31, 0x0e, 0x07, 0x8d, 0x3e, 0x34, 0x58, 0x83, 0xc1, 0x5e, 0xc3, 0x02, 0x04, 0x00, 0x01, 0xff, 0xff, 0x04, 0x00, 0x0e, 0x50, 0x61, 0x72, 0x73, 0x65, 0x20, 0x45, 0x72, 0x72, 0x6f, 0x72, 0x20, 0x25, 0x31, 0x0e, 0x07, 0x8d, 0x3e, 0x96, 0x58, 0x83, 0xc1, 0x1d, 0xc3, 0xff, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x16}

const unsigned short int pp_LSCAN = 0x66f9; // LSCAN size: 400
// {0x50, 0x4f, 0x4c, 0x59, 0x43, 0x4f, 0x4e, 0x20, 0x20, 0x20, 0x20, 0x20, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x03, 0x9c, 0x05, 0x01, 0x20, 0x00, 0x04, 0x01, 0x0b, 0x00, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x5c, 0x9c, 0x01, 0x00, 0x80, 0x01, 0x20, 0x12, 0x04, 0x02, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x53, 0x9c, 0x30, 0x3c, 0x30, 0x3c, 0x30, 0x3c, 0x30, 0x3c, 0x30, 0x3c, 0x04, 0x01}

const unsigned short int pp_W688B = 0x688b; // W688B size: 64
// {0xc2, 0x00, 0x3c, 0x00, 0xc2, 0x00, 0x30, 0x00, 0xc6, 0x00, 0x30, 0x00, 0xc6, 0x00, 0x3c, 0x00, 0x49, 0x41, 0x42, 0x4c, 0x45, 0x20, 0x2d, 0x20, 0x41, 0x52, 0x52, 0x41, 0x59, 0x20, 0x2d, 0x20, 0x53, 0x48, 0x5c, 0x20, 0x56, 0x41, 0x52, 0x20, 0x20, 0x20, 0x20, 0x20, 0x4b, 0x52, 0x4e, 0x20, 0x20, 0x72, 0x66, 0x67, 0x32, 0x32, 0x61, 0x75, 0x67, 0x38, 0x39, 0x20, 0x29, 0x2e, 0x04, 0x00}

const unsigned short int pp_W68CD = 0x68cd; // W68CD size: 64
// {0x3a, 0x20, 0x05, 0x41, 0x4c, 0x4c, 0x4f, 0x54, 0x20, 0x02, 0x0b, 0x00, 0x92, 0x63, 0x00, 0x00, 0x01, 0x10, 0x03, 0x03, 0x20, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x34, 0x0a, 0x08, 0x20, 0x34, 0x04, 0x01, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xa6, 0x0b, 0x0b, 0x20, 0x35, 0x04, 0x02, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xa4, 0x0a, 0x08, 0x20, 0x0b, 0x00, 0x00, 0x00}

const unsigned short int pp_W690F = 0x690f; // W690F size: 64
// {0x3a, 0x20, 0x05, 0x41, 0x4c, 0x4c, 0x4f, 0x54, 0x20, 0x41, 0x54, 0x48, 0x3d, 0x50, 0x52, 0x4f, 0x4d, 0x50, 0x54, 0x3d, 0x43, 0x4f, 0x4d, 0x53, 0x50, 0x45, 0x43, 0x3d, 0x04, 0x1b, 0x5b, 0x32, 0x4a, 0x42, 0xa8, 0x16, 0x44, 0xad, 0x2f, 0x45, 0x9c, 0x16, 0x47, 0xa0, 0x16, 0x48, 0x92, 0x16, 0x4c, 0xa4, 0x16, 0x4e, 0xb9, 0x16, 0x50, 0xc3, 0x16, 0x51, 0x98, 0x16, 0x54, 0x40, 0x28, 0x56}

const unsigned short int pp_W6951 = 0x6951; // W6951 size: 64
// {0x3a, 0x20, 0x05, 0x41, 0x4c, 0x4c, 0x4f, 0x54, 0x20, 0x4f, 0x54, 0x4e, 0x0a, 0x0a, 0x45, 0x52, 0x52, 0x4f, 0x52, 0x4c, 0x45, 0x56, 0x45, 0x4c, 0x12, 0x0b, 0x05, 0x45, 0x58, 0x49, 0x53, 0x54, 0xa5, 0x0a, 0x00, 0x03, 0x44, 0x49, 0x52, 0x03, 0xaf, 0x0f, 0x04, 0x43, 0x41, 0x4c, 0x4c, 0x02, 0x87, 0x0b, 0x04, 0x43, 0x48, 0x43, 0x50, 0x02, 0x0d, 0x19, 0x06, 0x52, 0x45, 0x4e, 0x41, 0x4d}

const unsigned short int pp_CMAP = 0x699a; // CMAP size: 64
// {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f}

const unsigned short int pp_ATIME = 0x69e4; // ATIME size: 4
// {0x70, 0x20, 0x61, 0x72}

const unsigned short int pp_LRTRN = 0x69f2; // LRTRN size: 4
// {0x48, 0x44, 0x49, 0x52}

const unsigned short int pp__ro_TRADER = 0x6a02; // (TRADER size: 4
// {0xec, 0x13, 0x00, 0x03}

const unsigned short int pp__ro_THING_rc_ = 0x6a12; // (THING) size: 4
// {0x49, 0x52, 0x03, 0x46}

const unsigned short int pp__n_ETIME = 0x6a31; // #ETIME size: 4
// {0x06, 0x50, 0x52, 0x4f}

const unsigned short int pp_NTIME = 0x6a3f; // NTIME size: 4
// {0x00, 0x00, 0x00, 0x00}

const unsigned short int pp__i_UHL = 0x6a4c; // 'UHL size: 2
// {0x48, 0x3a}

const unsigned short int pp__i_VERSIO = 0x6c5b; // 'VERSIO size: 2
// {0x1d, 0xaa}

const unsigned short int pp_CTTOP = 0x6c67; // CTTOP size: 2
// {0x32, 0x00}

const unsigned short int pp_CTBOT = 0x6c73; // CTBOT size: 2
// {0x02, 0x00}

const unsigned short int pp_GRSEG = 0x84f5; // GRSEG size: 2
// {0x3e, 0x95}

const unsigned short int pp_W8511 = 0x8511; // W8511 size: 5
// {0x00, 0x06, 0x05, 0x07, 0x03}

const unsigned short int pp_CCGA = 0x8622; // CCGA size: 16
// {0x00, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x03, 0x00, 0x01, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03}

const unsigned short int pp_PCGA = 0x863b; // PCGA size: 16
// {0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x00, 0x01, 0x01, 0x03, 0x01, 0x01, 0x03, 0x03}

const unsigned short int pp_W87B7 = 0x87b7; // W87B7 size: 4
// {0x3a, 0x20, 0x05, 0x41}

const unsigned short int pp_W89B2 = 0x89b2; // W89B2 size: 51
// {0x06, 0xff, 0x36, 0xf5, 0x84, 0x07, 0x26, 0x8b, 0x06, 0x08, 0x01, 0x8b, 0x0e, 0x56, 0x5a, 0x0b, 0xc9, 0x75, 0x05, 0x25, 0xff, 0x00, 0xeb, 0x03, 0x25, 0xff, 0x40, 0x26, 0x89, 0x06, 0x08, 0x01, 0x07, 0xb8, 0x05, 0x00, 0x8b, 0x0e, 0x6f, 0x59, 0x8b, 0x1e, 0x58, 0x59, 0x8b, 0x16, 0x63, 0x59, 0xcd, 0x63, 0xc3}

const unsigned short int pp_W89F9 = 0x89f9; // W89F9 size: 4
// {0x00, 0x04, 0x05, 0x05}

const unsigned short int pp_TREJECT = 0x8b5b; // TREJECT size: 6
// {0x9a, 0x41, 0x00, 0x37, 0x7f, 0xc3}

const unsigned short int pp_W8CAD = 0x8cad; // W8CAD size: 12
// {0x5b, 0x8b, 0x6d, 0x8b, 0x83, 0x8b, 0x99, 0x8b, 0xad, 0x8b, 0xbf, 0x8b}

const unsigned short int pp_SRC = 0x8dd9; // SRC size: 2
// {0x20, 0x33}

const unsigned short int pp_DST = 0x8de3; // DST size: 2
// {0x00, 0x83}

const unsigned short int pp_SEGME = 0x8def; // SEGME size: 2
// {0x20, 0x8d}

const unsigned short int pp_PLZ = 0x8df9; // PLZ size: 2
// {0x22, 0x5d}

const unsigned short int pp_XPCOL = 0x8e05; // XPCOL size: 2
// {0x00, 0x00}

const unsigned short int pp_NOF = 0xa489; // NOF size: 2
// {0x00, 0x00}

const unsigned short int pp__i_TRACE = 0xa583; // 'TRACE size: 2
// {0xa8, 0x49}

const unsigned short int pp_WAB57 = 0xab57; // WAB57 size: 2
// {0x06, 0x00}

const unsigned short int pp_SMART = 0xab63; // SMART size: 2
// {0x00, 0x00}

const unsigned short int pp_OTABL = 0xb63a; // OTABL size: 54
// {0x1c, 0x71, 0x01, 0xa7, 0x48, 0x01, 0x75, 0xd2, 0x00, 0x93, 0xd2, 0x00, 0xfd, 0xda, 0x00, 0x96, 0x2e, 0x01, 0x3a, 0x71, 0x01, 0x84, 0xfa, 0x00, 0xc7, 0xae, 0x00, 0x03, 0xaf, 0x00, 0x29, 0x20, 0x01, 0x20, 0x20, 0x20, 0xce, 0x9c, 0x00, 0x73, 0x97, 0x00, 0xa8, 0xab, 0x01, 0x6e, 0xae, 0x00, 0xe5, 0xae, 0x00, 0x8a, 0xab, 0x01}

const unsigned short int pp_BTADDR = 0xbae3; // BTADDR size: 2
// {0xfc, 0xbb}

const unsigned short int pp_HUFF_BT_POINTER = 0xbae7; // HUFF_BT_POINTER size: 2
// {0x3a, 0x20}

const unsigned short int pp_HUFF_ACTIVE_POINTER_BYTE = 0xbaeb; // HUFF_ACTIVE_POINTER_BYTE size: 2
// {0x3a, 0x20}

const unsigned short int pp_WBAEF = 0xbaef; // WBAEF size: 2
// {0x3a, 0x20}

const unsigned short int pp_HUFF_ACTIVE_POINTER_BIT = 0xbaf3; // HUFF_ACTIVE_POINTER_BIT size: 2
// {0x3a, 0x20}

const unsigned short int pp_HEALTI = 0xc128; // HEALTI size: 2
// {0x20, 0x45}

const unsigned short int pp_LASTAP = 0xc135; // LASTAP size: 4
// {0x49, 0x4e, 0x54, 0x20}

const unsigned short int pp_ROSTER = 0xc144; // ROSTER size: 18
// {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}

const unsigned short int pp__n_MISS = 0xc6e9; // #MISS size: 2
// {0x00, 0x00}


const unsigned short int cc_BL = 0x099a; // BL
const unsigned short int cc_C_slash_L = 0x09a4; // C/L
const unsigned short int cc_INIT_dash_FORTH = 0x09b5; // INIT-FORTH
const unsigned short int cc_INIT_dash_USER = 0x09c5; // INIT-USER
const unsigned short int cc_INITIAL_dash_DP = 0x09d6; // INITIAL-DP
const unsigned short int cc_LIMIT = 0x09e2; // LIMIT
const unsigned short int cc_MS_slash_TICK = 0x2a72; // MS/TICK
const unsigned short int cc_TICKS_slash_ADJ = 0x2a82; // TICKS/ADJ
const unsigned short int cc_MS_slash_ADJ = 0x2a8f; // MS/ADJ
const unsigned short int cc__n_DRIVES = 0x2b21; // #DRIVES
const unsigned short int cc__n_FCBS = 0x2b2d; // #FCBS
const unsigned short int cc__n_FILES = 0x2b3a; // #FILES
const unsigned short int cc__n_SEC_slash_TRK = 0x2b49; // #SEC/TRK
const unsigned short int cc_DOS_dash_FILE = 0x2b58; // DOS-FILE
const unsigned short int cc_RETRIES = 0x2b66; // RETRIES
const unsigned short int cc_SECORIGIN = 0x2b76; // SECORIGIN
const unsigned short int cc_PSW = 0x3991; // PSW
const unsigned short int cc_AX = 0x399a; // AX
const unsigned short int cc_BX = 0x39a3; // BX
const unsigned short int cc_CX = 0x39ac; // CX
const unsigned short int cc_DX = 0x39b5; // DX
const unsigned short int cc_BP = 0x39be; // BP
const unsigned short int cc_DI = 0x39c7; // DI
const unsigned short int cc_SI = 0x39d0; // SI
const unsigned short int cc_DS = 0x39d9; // DS
const unsigned short int cc_ES = 0x39e2; // ES
const unsigned short int cc__3 = 0x3b81; // 3
const unsigned short int cc__4 = 0x3b89; // 4
const unsigned short int cc__5 = 0x3b91; // 5
const unsigned short int cc__6 = 0x3b99; // 6
const unsigned short int cc__7 = 0x3ba1; // 7
const unsigned short int cc__8 = 0x3ba9; // 8
const unsigned short int cc__9 = 0x3bb1; // 9
const unsigned short int cc__dash_1 = 0x3bba; // -1
const unsigned short int cc__dash_2 = 0x3bc3; // -2
const unsigned short int cc__ro_C_c__rc_ = 0x3e3a; // (C:)
const unsigned short int cc_IHEADLE = 0x50f6; // IHEADLE
const unsigned short int cc_W50FA = 0x50fa; // W50FA
const unsigned short int cc_W50FE = 0x50fe; // W50FE
const unsigned short int cc_W5102 = 0x5102; // W5102
const unsigned short int cc__star_MAPSCA = 0x5110; // *MAPSCA
const unsigned short int cc_SIGBLK = 0x511d; // SIGBLK
const unsigned short int cc_ALOVSA = 0x512a; // ALOVSA
const unsigned short int cc_AHIVSA = 0x5137; // AHIVSA
const unsigned short int cc_BLOVSA = 0x5144; // BLOVSA
const unsigned short int cc_BHIVSA = 0x5151; // BHIVSA
const unsigned short int cc__i_OVBACK = 0x515f; // 'OVBACK
const unsigned short int cc_MUSSEG = 0x516c; // MUSSEG
const unsigned short int cc_IsFIRED_dash_ = 0x5184; // ?FIRED-
const unsigned short int cc_IsA_dash_SHIE = 0x5192; // ?A-SHIE
const unsigned short int cc_IsA_dash_WEAP = 0x51a0; // ?A-WEAP
const unsigned short int cc__16K = 0x51aa; // 16K
const unsigned short int cc_TRUE = 0x51c5; // TRUE
const unsigned short int cc_FALSE = 0x51d1; // FALSE
const unsigned short int cc_NULL_dash_IC = 0x522e; // NULL-IC
const unsigned short int cc_SYS_dash_ICO = 0x523c; // SYS-ICO
const unsigned short int cc_INVIS_dash_I = 0x524a; // INVIS-I
const unsigned short int cc_FLUX_dash_IC = 0x5258; // FLUX-IC
const unsigned short int cc_DEAD_dash_IC = 0x5266; // DEAD-IC
const unsigned short int cc_DEFAULT = 0x5274; // DEFAULT
const unsigned short int cc_TEXTC_slash_L = 0x5282; // TEXTC/L
const unsigned short int cc_W5286 = 0x5286; // W5286
const unsigned short int cc_FUEL_slash_SE = 0x5294; // FUEL/SE
const unsigned short int cc_POLYSEG = 0x52a2; // POLYSEG
const unsigned short int cc__i_ANSYS = 0x52af; // 'ANSYS
const unsigned short int cc_IsCALLED = 0x52bd; // ?CALLED
const unsigned short int cc_MPS = 0x53a9; // MPS
const unsigned short int cc__n_VECTOR = 0x8b4d; // #VECTOR
const unsigned short int cc_R2OVL = 0x8d8d; // R2OVL
const unsigned short int cc_WAB19 = 0xab19; // WAB19
const unsigned short int cc_WAB67 = 0xab67; // WAB67
const unsigned short int cc_WAB6B = 0xab6b; // WAB6B
const unsigned short int cc_WAB6F = 0xab6f; // WAB6F
const unsigned short int cc_WAB73 = 0xab73; // WAB73
const unsigned short int cc_WAB77 = 0xab77; // WAB77
const unsigned short int cc_WAB7B = 0xab7b; // WAB7B
const unsigned short int cc_WAB7F = 0xab7f; // WAB7F
const unsigned short int cc_SB = 0xb43a; // SB
const unsigned short int cc_HUFF_PHRASE_ADDCHAR = 0xbaf7; // HUFF_PHRASE_ADDCHAR
const unsigned short int cc_HUFF_PHRASE_INIT = 0xbafb; // HUFF_PHRASE_INIT
const unsigned short int cc_HUFF_PHRASE_FINISH = 0xbaff; // HUFF_PHRASE_FINISH

const unsigned short int user_SP0 = 0x078c; // SP0
const unsigned short int user_R0 = 0x078e; // R0
const unsigned short int user_DP = 0x0790; // DP
const unsigned short int user_FENCE = 0x0792; // FENCE
const unsigned short int user__ro_INTO_rc_ = 0x0794; // (INTO)
const unsigned short int user_MSGFLAGS = 0x0796; // MSGFLAGS
const unsigned short int user_VOC_dash_LINK = 0x0798; // VOC-LINK
const unsigned short int user_USERSIZE = 0x079a; // USERSIZE
const unsigned short int user_USERNEXT = 0x079c; // USERNEXT
const unsigned short int user__gt_IN = 0x079e; // >IN
const unsigned short int user_TIB = 0x07a0; // TIB
const unsigned short int user_VIDEOPORT = 0x07a2; // VIDEOPORT
const unsigned short int user_VIDEOMEM = 0x07a4; // VIDEOMEM
const unsigned short int user_CURSORLOC = 0x07a6; // CURSORLOC
const unsigned short int user_ROW = 0x07a6; // ROW
const unsigned short int user_COL = 0x07a7; // COL
const unsigned short int user_ROWMIN = 0x07a8; // ROWMIN
const unsigned short int user_COLMIN = 0x07a9; // COLMIN
const unsigned short int user_ROWMAX = 0x07aa; // ROWMAX
const unsigned short int user_COLMAX = 0x07ab; // COLMAX
const unsigned short int user_ATTRIBUTE = 0x07ac; // ATTRIBUTE
const unsigned short int user_DOFFSET = 0x07ae; // DOFFSET
const unsigned short int user_SERIALPORT = 0x07b2; // SERIALPORT
const unsigned short int user_PARALLELPORT = 0x07b4; // PARALLELPORT
const unsigned short int user_BASE = 0x07b6; // BASE
const unsigned short int user_BLK = 0x07b8; // BLK
const unsigned short int user_DPL = 0x07ba; // DPL
const unsigned short int user_FLD = 0x07bc; // FLD
const unsigned short int user_HLD = 0x07be; // HLD
const unsigned short int user_R_n_ = 0x07c0; // R#
const unsigned short int user_SCR = 0x07c2; // SCR
const unsigned short int user_SSCR = 0x07c4; // SSCR
const unsigned short int user_STATE = 0x07c6; // STATE
const unsigned short int user_CONTEXT_1 = 0x07c8; // CONTEXT_1
const unsigned short int user_CSP = 0x07ca; // CSP
const unsigned short int user_CURRENT = 0x07cc; // CURRENT
const unsigned short int user_LFALEN = 0x07ce; // LFALEN
const unsigned short int user_LOWERCASE = 0x07d0; // LOWERCASE
const unsigned short int user_SEARCHORDER = 0x07d2; // SEARCHORDER
const unsigned short int user_WIDTH = 0x07d4; // WIDTH

// 0x0100: cli    
// 0x0101: mov    ax,cs
// 0x0103: mov    ss,ax
// 0x0105: mov    es,ax
// 0x0107: mov    ds,ax
// 0x0109: mov    si,0129
// 0x010c: mov    di,[si+02]
// 0x010f: inc    di
// 0x0110: inc    di
// 0x0111: mov    bx,0705
// 0x0114: mov    bx,[bx]
// 0x0116: add    bx,0100
// 0x011a: mov    sp,bx
// 0x011c: add    bx,0080
// 0x0120: mov    bp,bx
// 0x0122: sti    
// 0x0123: cld    
// 0x0124: lodsw
// 0x0125: mov    bx,ax
// 0x0127: jmp    word ptr [bx]
// 0x0129: db 0x37 0x09 0x8a 0x07 '7   '

// ================================================
// 0x012d: WORD '(!SET)' codep=0x0138 wordp=0x0138 params=3 returns=0
// ================================================
// 0x0138: mov    dx,ds
// 0x013a: mov    ax,0000
// 0x013d: mov    ds,ax
// 0x013f: pop    bx
// 0x0140: add    bx,bx
// 0x0142: add    bx,bx
// 0x0144: cli    
// 0x0145: pop    ax
// 0x0146: mov    [bx],ax
// 0x0148: inc    bx
// 0x0149: inc    bx
// 0x014a: pop    ax
// 0x014b: mov    [bx],ax
// 0x014d: sti    
// 0x014e: mov    ds,dx
// 0x0150: lodsw
// 0x0151: mov    bx,ax
// 0x0153: jmp    word ptr [bx]

// ================================================
// 0x0155: WORD '(!OLD)' codep=0x0160 wordp=0x0160 params=1 returns=2
// ================================================
// 0x0160: mov    dx,ds
// 0x0162: mov    ax,0000
// 0x0165: mov    ds,ax
// 0x0167: pop    bx
// 0x0168: add    bx,bx
// 0x016a: add    bx,bx
// 0x016c: cli    
// 0x016d: mov    ax,[bx]
// 0x016f: inc    bx
// 0x0170: inc    bx
// 0x0171: mov    bx,[bx]
// 0x0173: sti    
// 0x0174: push   bx
// 0x0175: push   ax
// 0x0176: mov    ds,dx
// 0x0178: lodsw
// 0x0179: mov    bx,ax
// 0x017b: jmp    word ptr [bx]

// ================================================
// 0x017d: WORD '(TIME)' codep=0x1d29 wordp=0x0188
// ================================================
// 0x0188: db 0x00 0x00 0xbf 0x35 0x08 0x00 0x2e 0x81 0x06 0x8a 0x01 0x37 0x00 0x2e 0x83 0x16 0x88 0x01 0x00 0x2e 0xc7 0x06 0x93 0x01 0x37 0x00 0x2e 0xff 0x0e 0x8c 0x01 0x75 0x0e 0x2e 0xc7 0x06 0x8c 0x01 0x0f 0x00 0x2e 0xc7 0x06 0x93 0x01 0x36 0x00 0xcf 0x00 0x00 0x00 0x00 0x80 0x05 0x8b 0x2c 0x62 0x66 0xc4 0x02 0x58 0x40 0x50 0x29 0xc0 0x29 0xd2 0xcf '   5  .    7 .     .    7 .    u .      .    6         ,bf  X@P) )  '
// 0x01cc: xor    bx,bx
// 0x01ce: div    bx
// 0x01d0: ret    

// 0x01d1: pop    ax
// 0x01d2: mov    cx,ax
// 0x01d4: sub    ax,01D0
// 0x01d8: jnz    01E0
// 0x01da: mov    ax,01C7
// 0x01dd: jmp    01E4
// 0x01e0: mov    ax,01C4
// 0x01e3: inc    cx
// 0x01e4: mov    dx,ds
// 0x01e6: xor    bx,bx
// 0x01e8: mov    ds,bx
// 0x01ea: mov    [bx],ax
// 0x01ec: mov    ds,dx
// 0x01ee: push   cx
// 0x01ef: iret   

// ================================================
// 0x01f0: WORD 'SET0/' codep=0x01fa wordp=0x01fa
// ================================================
// 0x01fa: mov    ax,ds
// 0x01fc: xor    bx,bx
// 0x01fe: mov    ds,bx
// 0x0200: mov    word ptr [bx],01D1
// 0x0204: add    bx,0002
// 0x0208: mov    [bx],ax
// 0x020a: mov    ds,ax
// 0x020c: call   01CC
// 0x020f: lodsw
// 0x0210: mov    bx,ax
// 0x0212: jmp    word ptr [bx]

// ================================================
// 0x0214: WORD '(RESTORE)' codep=0x224c wordp=0x0222 params=0 returns=0
// ================================================

void _ro_RESTORE_rc_() // (RESTORE)
{
  Push(0x01b8);
  _2_at_(); // 2@
  Push(Pop() | Pop()); // OR
  if (Pop() == 0) return;
  Push(0x01b8);
  _2_at_(); // 2@
  Push(0x001b);
  _ro__ex_SET_rc_(); // (!SET)
  Push(0x01bc);
  _2_at_(); // 2@
  Push(0x001c);
  _ro__ex_SET_rc_(); // (!SET)
  Push(0x01c0);
  _2_at_(); // 2@
  Push(0);
  _ro__ex_SET_rc_(); // (!SET)
  Push(0);
  Push(0);
  Push(0x01b8);
  _2_ex__1(); // 2!_1
  Push(Read16(pp__i_RESTORE_plus_)); // 'RESTORE+ @
  if (Read16(regsp) != 0) Push(Read16(regsp)); // ?DUP
  if (Pop() == 0) return;
  EXECUTE(); // EXECUTE
}


// ================================================
// 0x026a: WORD 'AUTO-CACHE' codep=0x224c wordp=0x0279 params=0 returns=0
// ================================================

void AUTO_dash_CACHE() // AUTO-CACHE
{
  Push(Read16(pp__n_CACHE)); // #CACHE @
  if (Pop() == 0) return;
  Push(Read16(pp_SYSK) * 0x0040); // SYSK @ 0x0040 *
  Push(Read16(cc_LIMIT)); // LIMIT
  ADDR_gt_SEG(); // ADDR>SEG
  Push(Pop() + 1); //  1+
  Push(Read16(regsp)); // DUP
  Push(pp_BLKCACHE); // BLKCACHE
  Store(); // !
  _dash_(); // -
  Push(0);
  Push(0x0042);
  U_slash_MOD(); // U/MOD
  SWAP(); // SWAP
  Pop(); // DROP
  Push(Read16(pp__n_CACHE)); // #CACHE @
  UMIN(); // UMIN
  Push(pp__n_CACHE); // #CACHE
  Store(); // !
  INITCACHE(); // INITCACHE
}


// ================================================
// 0x02b3: WORD 'AUTO-LIMIT' codep=0x224c wordp=0x02c2 params=0 returns=0
// ================================================

void AUTO_dash_LIMIT() // AUTO-LIMIT
{
  Push(Read16(cc_LIMIT)); // LIMIT
  Push(-2);
  NOP(); // NOP
  OVER(); // OVER
  if (Read16(regsp) != 0) Push(Read16(regsp)); // ?DUP
  if (Pop() != 0)
  {
    UMIN(); // UMIN
  }
  Push(0x09e2); // 'LIMIT'
  Store(); // !
  Push(Pop() - Read16(cc_LIMIT)); //  LIMIT -
  if (Pop() == 0) return;
  CHANGE(); // CHANGE
}


// ================================================
// 0x02e6: WORD 'AUTO-SCREEN' codep=0x224c wordp=0x02f6 params=0 returns=0
// ================================================

void AUTO_dash_SCREEN() // AUTO-SCREEN
{
  Push(0x0f00);
  Push(Read16(cc_AX)); // AX
  Store(); // !
  Push(0x0010);
  INTERRUPT(); // INTERRUPT
  Push((Read16(Read16(cc_AX))&0xFF)==7?1:0); // AX C@ 7 =
  if (Pop() != 0)
  {
    Push(0x03b4);
    Push(0xb000);
  } else
  {
    Push(0x03d4);
    Push(0xb800);
  }
  Push(user_VIDEOMEM); // VIDEOMEM
  Store(); // !
  Push(user_VIDEOPORT); // VIDEOPORT
  Store(); // !
}


// ================================================
// 0x0330: WORD 'COLD' codep=0x224c wordp=0x0339 params=0 returns=0
// ================================================

void COLD() // COLD
{
  EMPTY(); // EMPTY
  Exec("CR"); // call of word 0x26ee '(CR)'
  W0939(); // W0939
}


// ================================================
// 0x0341: WORD 'W0343' codep=0x224c wordp=0x0343 params=0 returns=0
// ================================================

void W0343() // W0343
{
  PRINT("FORTHOUGHT (P) (C) Copyright ", 29); // (.")
  PRINT("Fantasia Systems Inc. 1982,1983,1984", 36); // (.")
  Exec("CR"); // call of word 0x26ee '(CR)'
  PRINT("Version 2.52 for Binary Systems for the IBM PC", 46); // (.")
  Exec("CR"); // call of word 0x26ee '(CR)'
}


// ================================================
// 0x03c1: WORD 'W03C3' codep=0x224c wordp=0x03c3 params=0 returns=0
// ================================================

void W03C3() // W03C3
{
  PRINT(" ok", 3); // (.")
}


// ================================================
// 0x03cb: WORD 'SET-BUFFERS' codep=0x224c wordp=0x03db params=0 returns=0
// ================================================

void SET_dash_BUFFERS() // SET-BUFFERS
{
  Push(Read16(cc_LIMIT) - 0x0820); // LIMIT 0x0820 -
  ADDR_gt_SEG(); // ADDR>SEG
  Push(Read16(regsp)); // DUP
  Push(pp_USE); // USE
  Store(); // !
  Push(Read16(regsp)); // DUP
  Push(pp_BUFFER_dash_BEGIN); // BUFFER-BEGIN
  Store(); // !
  Push(Pop() + 0x0041); //  0x0041 +
  Push(Read16(regsp)); // DUP
  Push(pp_LPREV); // LPREV
  Store(); // !
  Push(pp_PREV); // PREV
  Store(); // !
  MTBUFFERS(); // MTBUFFERS
}


// ================================================
// 0x0405: WORD 'SP0' codep=0x1792 wordp=0x040d
// ================================================
// 0x040d: db 0x00 0x00 '  '

// ================================================
// 0x040f: WORD 'R0' codep=0x1792 wordp=0x0416
// ================================================
// 0x0416: db 0x02 0x00 '  '

// ================================================
// 0x0418: WORD 'DP' codep=0x1792 wordp=0x041f
// ================================================
// 0x041f: db 0x04 0x00 '  '

// ================================================
// 0x0421: WORD 'FENCE' codep=0x1792 wordp=0x042b
// ================================================
// 0x042b: db 0x06 0x00 '  '

// ================================================
// 0x042d: WORD '(INTO)' codep=0x1792 wordp=0x0438
// ================================================
// 0x0438: db 0x08 0x00 '  '

// ================================================
// 0x043a: WORD 'MSGFLAGS' codep=0x1792 wordp=0x0447
// ================================================
// 0x0447: db 0x0a 0x00 '  '

// ================================================
// 0x0449: WORD 'VOC-LINK' codep=0x1792 wordp=0x0456
// ================================================
// 0x0456: db 0x0c 0x00 '  '

// ================================================
// 0x0458: WORD 'USERSIZE' codep=0x1792 wordp=0x0465
// ================================================
// 0x0465: db 0x0e 0x00 '  '

// ================================================
// 0x0467: WORD 'USERNEXT' codep=0x1792 wordp=0x0474
// ================================================
// 0x0474: db 0x10 0x00 '  '

// ================================================
// 0x0476: WORD '>IN' codep=0x1792 wordp=0x047e
// ================================================
// 0x047e: db 0x12 0x00 '  '

// ================================================
// 0x0480: WORD 'TIB' codep=0x1792 wordp=0x0488
// ================================================
// 0x0488: db 0x14 0x00 '  '

// ================================================
// 0x048a: WORD 'VIDEOPORT' codep=0x1792 wordp=0x0498
// ================================================
// 0x0498: db 0x16 0x00 '  '

// ================================================
// 0x049a: WORD 'VIDEOMEM' codep=0x1792 wordp=0x04a7
// ================================================
// 0x04a7: db 0x18 0x00 '  '

// ================================================
// 0x04a9: WORD 'CURSORLOC' codep=0x1792 wordp=0x04b7
// ================================================
// 0x04b7: db 0x1a 0x00 '  '

// ================================================
// 0x04b9: WORD 'ROW' codep=0x1792 wordp=0x04c1
// ================================================
// 0x04c1: db 0x1a 0x00 '  '

// ================================================
// 0x04c3: WORD 'COL' codep=0x1792 wordp=0x04cb
// ================================================
// 0x04cb: db 0x1b 0x00 '  '

// ================================================
// 0x04cd: WORD 'ROWMIN' codep=0x1792 wordp=0x04d8
// ================================================
// 0x04d8: db 0x1c 0x00 '  '

// ================================================
// 0x04da: WORD 'COLMIN' codep=0x1792 wordp=0x04e5
// ================================================
// 0x04e5: db 0x1d 0x00 '  '

// ================================================
// 0x04e7: WORD 'ROWMAX' codep=0x1792 wordp=0x04f2
// ================================================
// 0x04f2: db 0x1e 0x00 '  '

// ================================================
// 0x04f4: WORD 'COLMAX' codep=0x1792 wordp=0x04ff
// ================================================
// 0x04ff: db 0x1f 0x00 '  '

// ================================================
// 0x0501: WORD 'ATTRIBUTE' codep=0x1792 wordp=0x050f
// ================================================
// 0x050f: db 0x20 0x00 '  '

// ================================================
// 0x0511: WORD 'DOFFSET' codep=0x1792 wordp=0x051d
// ================================================
// 0x051d: db 0x22 0x00 '" '

// ================================================
// 0x051f: WORD 'SERIALPORT' codep=0x1792 wordp=0x052e
// ================================================
// 0x052e: db 0x26 0x00 '& '

// ================================================
// 0x0530: WORD 'PARALLELPORT' codep=0x1792 wordp=0x0541
// ================================================
// 0x0541: db 0x28 0x00 '( '

// ================================================
// 0x0543: WORD 'BASE' codep=0x1792 wordp=0x054c
// ================================================
// 0x054c: db 0x2a 0x00 '* '

// ================================================
// 0x054e: WORD 'BLK' codep=0x1792 wordp=0x0556
// ================================================
// 0x0556: db 0x2c 0x00 ', '

// ================================================
// 0x0558: WORD 'DPL' codep=0x1792 wordp=0x0560
// ================================================
// 0x0560: db 0x2e 0x00 '. '

// ================================================
// 0x0562: WORD 'FLD' codep=0x1792 wordp=0x056a
// ================================================
// 0x056a: db 0x30 0x00 '0 '

// ================================================
// 0x056c: WORD 'HLD' codep=0x1792 wordp=0x0574
// ================================================
// 0x0574: db 0x32 0x00 '2 '

// ================================================
// 0x0576: WORD 'R#' codep=0x1792 wordp=0x057d
// ================================================
// 0x057d: db 0x34 0x00 '4 '

// ================================================
// 0x057f: WORD 'SCR' codep=0x1792 wordp=0x0587
// ================================================
// 0x0587: db 0x36 0x00 '6 '

// ================================================
// 0x0589: WORD 'SSCR' codep=0x1792 wordp=0x0592
// ================================================
// 0x0592: db 0x38 0x00 '8 '

// ================================================
// 0x0594: WORD 'STATE' codep=0x1792 wordp=0x059e
// ================================================
// 0x059e: db 0x3a 0x00 ': '

// ================================================
// 0x05a0: WORD 'CONTEXT_1' codep=0x1792 wordp=0x05ac
// ================================================
// 0x05ac: db 0x3c 0x00 '< '

// ================================================
// 0x05ae: WORD 'CSP' codep=0x1792 wordp=0x05b6
// ================================================
// 0x05b6: db 0x3e 0x00 '> '

// ================================================
// 0x05b8: WORD 'CURRENT' codep=0x1792 wordp=0x05c4
// ================================================
// 0x05c4: db 0x40 0x00 '@ '

// ================================================
// 0x05c6: WORD 'LFALEN' codep=0x1792 wordp=0x05d1
// ================================================
// 0x05d1: db 0x42 0x00 'B '

// ================================================
// 0x05d3: WORD 'LOWERCASE' codep=0x1792 wordp=0x05e1
// ================================================
// 0x05e1: db 0x44 0x00 'D '

// ================================================
// 0x05e3: WORD 'SEARCHORDER' codep=0x1792 wordp=0x05f3
// ================================================
// 0x05f3: db 0x46 0x00 'F '

// ================================================
// 0x05f5: WORD 'WIDTH' codep=0x1792 wordp=0x05ff
// ================================================
// 0x05ff: db 0x48 0x00 'H '

// ================================================
// 0x0601: WORD '-FIND' codep=0x17b7 wordp=0x060b
// ================================================
// 0x060b: dw 0x004a

// ================================================
// 0x060d: WORD '?TERMINAL' codep=0x17b7 wordp=0x061b
// ================================================
// 0x061b: dw 0x004c

// ================================================
// 0x061d: WORD 'ABORT' codep=0x17b7 wordp=0x0627
// ================================================
// 0x0627: dw 0x004e

// ================================================
// 0x0629: WORD 'BELL' codep=0x17b7 wordp=0x0632
// ================================================
// 0x0632: dw 0x0050

// ================================================
// 0x0634: WORD 'BS' codep=0x17b7 wordp=0x063b
// ================================================
// 0x063b: dw 0x0052

// ================================================
// 0x063d: WORD 'CR' codep=0x17b7 wordp=0x0644
// ================================================
// 0x0644: dw 0x0054

// ================================================
// 0x0646: WORD 'CREATE' codep=0x17b7 wordp=0x0651
// ================================================
// 0x0651: dw 0x0056

// ================================================
// 0x0653: WORD 'DISKERROR?' codep=0x17b7 wordp=0x0662
// ================================================
// 0x0662: dw 0x0058

// ================================================
// 0x0664: WORD 'EMIT' codep=0x17b7 wordp=0x066d
// ================================================
// 0x066d: dw 0x005a

// ================================================
// 0x066f: WORD 'EXPECT' codep=0x17b7 wordp=0x067a
// ================================================
// 0x067a: dw 0x005c

// ================================================
// 0x067c: WORD 'INTERPRET' codep=0x17b7 wordp=0x068a
// ================================================
// 0x068a: dw 0x005e

// ================================================
// 0x068c: WORD 'LINKS>' codep=0x17b7 wordp=0x0697
// ================================================
// 0x0697: dw 0x0060

// ================================================
// 0x0699: WORD 'LOAD_1' codep=0x17b7 wordp=0x06a2
// ================================================
// 0x06a2: dw 0x0062

// ================================================
// 0x06a4: WORD 'KEY_1' codep=0x17b7 wordp=0x06ac
// ================================================
// 0x06ac: dw 0x0064

// ================================================
// 0x06ae: WORD 'NUMBER' codep=0x17b7 wordp=0x06b9
// ================================================
// 0x06b9: dw 0x0066

// ================================================
// 0x06bb: WORD 'PAGE' codep=0x17b7 wordp=0x06c4
// ================================================
// 0x06c4: dw 0x0068

// ================================================
// 0x06c6: WORD 'POSITION' codep=0x17b7 wordp=0x06d3
// ================================================
// 0x06d3: dw 0x006a

// ================================================
// 0x06d5: WORD 'R/W' codep=0x17b7 wordp=0x06dd
// ================================================
// 0x06dd: dw 0x006c

// ================================================
// 0x06df: WORD 'TYPE' codep=0x17b7 wordp=0x06e8
// ================================================
// 0x06e8: dw 0x006e

// ================================================
// 0x06ea: WORD 'WHERE' codep=0x17b7 wordp=0x06f4
// ================================================
// 0x06f4: dw 0x0070

// ================================================
// 0x06f6: WORD 'WORD' codep=0x17b7 wordp=0x06ff
// ================================================
// 0x06ff: dw 0x0072
// 0x0701: db 0xf4 0xf6 0xd0 0xf7 0xf0 0xd5 0xe6 0x39 0x00 0x00 0x31 0x00 0x27 0xd5 0x84 0x00 0x74 0x00 0x00 0x00 0xf4 0xf6 0xd4 0x03 0x00 0xb8 0x18 0x00 0x00 0x00 0x18 0x4f 0x00 0x07 0xff 0xff 0xff 0xff 0xf8 0x03 0xbc 0x03 0x0a 0x00 0x00 0x00 0xff 0xff 0x00 0x00 0x81 0xd7 0x00 0x00 0x82 0x01 0x03 0x00 0x00 0x00 0x1a 0x08 0xf4 0xf6 0x1a 0x08 0x02 0x00 0x00 0x00 0x00 0x00 0x1f 0x00 0xe1 0x17 0xbc 0x25 0x45 0x1c 0x6b 0x26 0xcf 0x26 0xee 0x26 0xbb 0x1c 0x48 0x36 0x31 0x27 0x3e 0x1d 0xd8 0x1d 0x5c 0x1a 0x23 0x1e 0xd7 0x25 0xd1 0x13 0x4c 0x27 0x67 0x27 0xd7 0x36 0x90 0x26 0x68 0x1e 0x06 0x1f 0xc2 0xd5 0x6a 0xd5 0xc2 0xd5 0x49 0xd5 0xb2 0xd5 '       9  1 '   t              O                                             %E k& & &  H61'>   \ #  %  L'g' 6 &h     j   I   '

// ================================================
// 0x077f: WORD 'OPERATOR' codep=0x1d29 wordp=0x078c
// ================================================
// 0x078c: db 0xf4 0xf6 0xd0 0xf7 0xf0 0xd5 0xe6 0x39 0x00 0x00 0x31 0x00 0x27 0xd5 0x84 0x00 0x74 0x00 0x0c 0x00 0xf4 0xf6 0xd4 0x03 0x00 0xb8 0x18 0x00 0x00 0x00 0x18 0x4f 0x00 0x07 0xff 0xff 0xff 0xff 0xf8 0x03 0xbc 0x03 0x0a 0x00 0x00 0x00 0xff 0xff 0x00 0x00 0x81 0xd7 0x00 0x00 0x82 0x01 0x03 0x00 0x00 0x00 0x1a 0x08 0xf4 0xf6 0x1a 0x08 0x02 0x00 0x00 0x00 0x00 0x00 0x1f 0x00 0xe1 0x17 0xbc 0x25 0x45 0x1c 0x6b 0x26 0xcf 0x26 0xee 0x26 0xbb 0x1c 0x48 0x36 0x31 0x27 0x3e 0x1d 0xd8 0x1d 0x5c 0x1a 0x23 0x1e 0xd7 0x25 0xd1 0x13 0x4c 0x27 0x67 0x27 0xd7 0x36 0x90 0x26 0x68 0x1e 0x06 0x1f 0xa9 0x0f 0x75 0x95 0xb1 0x24 0xa9 0x7b 0x07 0xc0 0xab 0xbc 0xb1 0xad 0xb4 0x20 '       9  1 '   t              O                                             %E k& & &  H61'>   \ #  %  L'g' 6 &h     u  $ {        '

// ================================================
// 0x0810: WORD 'FORTH' codep=0x1ab5 wordp=0x081a
// ================================================
// 0x081a: db 0x00 0x00 0x00 0x00 0xc2 0xd5 0x6a 0xd5 0xc2 0xd5 0x49 0xd5 0xb2 0xd5 '      j   I   '

// ================================================
// 0x0828: WORD 'FREEZE' codep=0x224c wordp=0x0833 params=0 returns=0
// ================================================

void FREEZE() // FREEZE
{
  Push(pp_OPERATOR); // OPERATOR
  Push(0x0701);
  Push(0x0074);
  CMOVE_1(); // CMOVE_1
  Push(0x081a); // 'FORTH'
  Push(Pop() + 4); //  4 +
  Push(0x0775);
  Push(0x000a);
  CMOVE_1(); // CMOVE_1
}


// ================================================
// 0x0855: WORD 'BYE_1' codep=0x224c wordp=0x085d params=0 returns=0
// ================================================

void BYE_1() // BYE_1
{
  FREEZE(); // FREEZE
  _ro_RESTORE_rc_(); // (RESTORE)
  Push(0);
  GO(); // GO
}


// ================================================
// 0x0867: WORD 'CHANGE' codep=0x224c wordp=0x0872 params=0 returns=0
// ================================================

void CHANGE() // CHANGE
{
  FREEZE(); // FREEZE
  SET_dash_BUFFERS(); // SET-BUFFERS
  Push(Read16(pp_BUFFER_dash_BEGIN)); // BUFFER-BEGIN @
  SEG_gt_ADDR(); // SEG>ADDR
  Push(Read16(regsp)); // DUP
  Push(Pop() - 0x00dc); //  0x00dc -
  Push(Read16(regsp)); // DUP
  Push(0x0701);
  Store(); // !
  Push(0x0715);
  Store(); // !
  Push(0x0703);
  Store(); // !
  Push(0x0100);
  GO(); // GO
}

// 0x08a0: db 0xfa 0xb8 0x40 0x00 0x8e 0xd8 0xbb 0x71 0x00 0xc6 0x07 0x00 0xb0 0x20 0xe6 0x20 0x8c 0xc8 0x8e 0xd0 0x8e 0xc0 0x8e 0xd8 0xb8 0x8c 0x07 0x8b 0xf8 0xb8 0x43 0x0a 0x8b 0xf0 0x8b 0x26 0x01 0x07 0x8b 0x2e 0x03 0x07 0xfc 0xfb 0xad 0x8b 0xd8 0xff 0x27 '  @    q                      C    &   .        ''

// ================================================
// 0x08d1: WORD '(SETUP)' codep=0x224c wordp=0x08dd params=0 returns=0
// ================================================

void _ro_SETUP_rc_() // (SETUP)
{
  Push(0x01b8);
  _2_at_(); // 2@
  Push(Pop() | Pop()); // OR
  Push(Pop()==0?1:0); //  0=
  if (Pop() == 0) return;
  Push(0x001b);
  _ro__ex_OLD_rc_(); // (!OLD)
  Push(0x01b8);
  _2_ex__1(); // 2!_1
  _ro_CS_ask__rc_(); // (CS?)
  Push(0x08a0);
  Push(0x001b);
  _ro__ex_SET_rc_(); // (!SET)
  Push(0x001c);
  _ro__ex_OLD_rc_(); // (!OLD)
  Push(0x01bc);
  _2_ex__1(); // 2!_1
  _ro_CS_ask__rc_(); // (CS?)
  Push(0x018e);
  Push(0x001c);
  _ro__ex_SET_rc_(); // (!SET)
  Push(0);
  _ro__ex_OLD_rc_(); // (!OLD)
  Push(0x01c0);
  _2_ex__1(); // 2!_1
  SET0_slash_(); // SET0/
  Push(Read16(pp__i_SETUP_plus_)); // 'SETUP+ @
  if (Read16(regsp) != 0) Push(Read16(regsp)); // ?DUP
  if (Pop() == 0) return;
  EXECUTE(); // EXECUTE
}


// ================================================
// 0x0937: WORD 'W0939' codep=0x224c wordp=0x0939 params=0 returns=0
// ================================================

void W0939() // W0939
{
  _ro_SETUP_rc_(); // (SETUP)
  _ro_CS_ask__rc_(); // (CS?)
  Push(Read16(cc_DS)); // DS
  Store(); // !
  Push(0xf2d2);
  Push(Read16(cc_PSW)); // PSW
  Store(); // !
  Push(0x0701);
  Push(pp_OPERATOR); // OPERATOR
  Push(0x0074);
  CMOVE_1(); // CMOVE_1
  Push(Read16(user_DP)); // DP @
  Push(Read16(cc_INITIAL_dash_DP)); // INITIAL-DP
  Store(); // !
  _i_FORTH(); // 'FORTH
  Push(user_CONTEXT_1); // CONTEXT_1
  Store(); // !
  DEFINITIONS(); // DEFINITIONS
  FREEZE(); // FREEZE
  AUTO_dash_LIMIT(); // AUTO-LIMIT
  AUTO_dash_CACHE(); // AUTO-CACHE
  SP_ex_(); // SP!
  RP_ex_(); // RP!
  SET_dash_BUFFERS(); // SET-BUFFERS
  AUTO_dash_SCREEN(); // AUTO-SCREEN
  W0343(); // W0343
  Push(Read16(pp_BOOT_dash_HOOK)); // BOOT-HOOK @
  if (Read16(regsp) != 0) Push(Read16(regsp)); // ?DUP
  if (Pop() != 0)
  {
    EXECUTE(); // EXECUTE
  }
  Push(Read16(pp_BOOT_dash_LOAD)); // BOOT-LOAD @
  if (Read16(regsp) != 0) Push(Read16(regsp)); // ?DUP
  if (Pop() != 0)
  {
    Exec("LOAD_1"); // call of word 0x1e23 '(LOAD)'
  }
  W03C3(); // W03C3
  Exec("ABORT"); // call of word 0x1c45 '(ABORT)'
}


// ================================================
// 0x0993: WORD 'BL' codep=0x2214 wordp=0x099a
// ================================================
// 0x099a: dw 0x0020

// ================================================
// 0x099c: WORD 'C/L' codep=0x2214 wordp=0x09a4
// ================================================
// 0x09a4: dw 0x0040

// ================================================
// 0x09a6: WORD 'INIT-FORTH' codep=0x2214 wordp=0x09b5
// ================================================
// 0x09b5: dw 0x0775

// ================================================
// 0x09b7: WORD 'INIT-USER' codep=0x2214 wordp=0x09c5
// ================================================
// 0x09c5: dw 0x0701

// ================================================
// 0x09c7: WORD 'INITIAL-DP' codep=0x2214 wordp=0x09d6
// ================================================
// 0x09d6: dw 0x0705

// ================================================
// 0x09d8: WORD 'LIMIT' codep=0x2214 wordp=0x09e2
// ================================================
// 0x09e2: dw 0xfffe

// ================================================
// 0x09e4: WORD '#CACHE' codep=0x1d29 wordp=0x09ef
// ================================================
// 0x09ef: db 0x99 0x00 '  '

// ================================================
// 0x09f1: WORD '#BUFFERS' codep=0x1d29 wordp=0x09fe
// ================================================
// 0x09fe: db 0x02 0x00 '  '

// ================================================
// 0x0a00: WORD '#SPACE' codep=0x1d29 wordp=0x0a0b
// ================================================
// 0x0a0b: db 0x00 0x01 '  '

// ================================================
// 0x0a0d: WORD ''RESTORE+' codep=0x1d29 wordp=0x0a1b
// ================================================
// 0x0a1b: db 0xbb 0xb2 '  '

// ================================================
// 0x0a1d: WORD ''SETUP+' codep=0x1d29 wordp=0x0a29
// ================================================
// 0x0a29: db 0x8b 0xb2 '  '

// ================================================
// 0x0a2b: WORD '@,0*1;' codep=0x1d29 wordp=0x0a36
// ================================================
// 0x0a36: db 0x01 0x00 '  '

// ================================================
// 0x0a38: WORD '>BREAK' codep=0x1d29 wordp=0x0a43
// ================================================
// 0x0a43: db 0x2c 0x16 ', '

// ================================================
// 0x0a45: WORD 'BOOT-HOOK' codep=0x1d29 wordp=0x0a53
// ================================================
// 0x0a53: db 0xcc 0xd5 '  '

// ================================================
// 0x0a55: WORD 'BOOT-LOAD' codep=0x1d29 wordp=0x0a63
// ================================================
// 0x0a63: db 0x00 0x00 '  '

// ================================================
// 0x0a65: WORD 'BUFFER-BEGIN' codep=0x1d29 wordp=0x0a76
// ================================================
// 0x0a76: db 0xe0 0x42 ' B'

// ================================================
// 0x0a78: WORD 'BUFFER-HEAD' codep=0x1d29 wordp=0x0a88
// ================================================
// 0x0a88: db 0x00 0x00 '  '

// ================================================
// 0x0a8a: WORD 'CACHE-BEGIN' codep=0x1d29 wordp=0x0a9a
// ================================================
// 0x0a9a: db 0x00 0x00 '  '

// ================================================
// 0x0a9c: WORD 'CACHE-HEAD' codep=0x1d29 wordp=0x0aab
// ================================================
// 0x0aab: db 0x00 0x00 '  '

// ================================================
// 0x0aad: WORD ''FORTH' codep=0x224c wordp=0x0ab8 params=0 returns=1
// ================================================

void _i_FORTH() // 'FORTH
{
  Push(Read16(user_USERSIZE)); // USERSIZE @
  ME(); // ME
  Push(Pop() + Pop()); // +
  Push(Pop() + Read16(user_LFALEN)); //  LFALEN @ +
  PFA(); // PFA
}


// ================================================
// 0x0aca: WORD 'ME' codep=0x0ad1 wordp=0x0ad1 params=0 returns=1
// ================================================
// 0x0ad1: push   di
// 0x0ad2: lodsw
// 0x0ad3: mov    bx,ax
// 0x0ad5: jmp    word ptr [bx]

// ================================================
// 0x0ad7: WORD 'W0AD9' codep=0x224c wordp=0x0ad9 params=1 returns=0
// ================================================

void W0AD9() // W0AD9
{
  Push(0);
  Push(Read16(pp__n_SPACE)); // #SPACE @
  Push(0);
  D_plus_(); // D+
  HERE(); // HERE
  Push(0);
  D_plus_(); // D+
  SWAP(); // SWAP
  SP_at_(); // SP@
  SWAP(); // SWAP
  U_st_(); // U<
  Push(Pop() | Pop()); // OR
  ABORT("Dictionary or stack full", 24);// (ABORT")
}


// ================================================
// 0x0b10: WORD 'W0B12' codep=0x224c wordp=0x0b12 params=0 returns=0
// ================================================
// orphan

void W0B12() // W0B12
{
  SP_at_(); // SP@
  S0(); // S0
  SWAP(); // SWAP
  U_st_(); // U<
  ABORT("Empty stack", 11);// (ABORT")
  Push(0);
  W0AD9(); // W0AD9
}


// ================================================
// 0x0b2e: WORD 'W0B30' codep=0x224c wordp=0x0b30 params=1 returns=0
// ================================================

void W0B30() // W0B30
{
  Push(Pop() & (((Read16(user_MSGFLAGS) & 8)==0?1:0)==0?1:0)); //  MSGFLAGS @ 8 AND 0= 0= AND
  ABORT("Input stream exhausted", 22);// (ABORT")
}


// ================================================
// 0x0b5b: WORD 'W0B5D' codep=0x224c wordp=0x0b5d params=1 returns=0
// ================================================

void W0B5D() // W0B5D
{
  ABORT("Not recognized", 14);// (ABORT")
}


// ================================================
// 0x0b70: WORD '?ENOUGH' codep=0x224c wordp=0x0b7c params=1 returns=0
// ================================================

void IsENOUGH() // ?ENOUGH
{
  DEPTH(); // DEPTH
  Push(Read16(regsp)); // DUP
  if (Pop() != 0)
  {
    _st_(); // <
  }
  Push(Pop()==0?1:0); //  0=
  ABORT("Not enough items on the stack", 29);// (ABORT")
}


// ================================================
// 0x0baa: WORD '@' codep=0x0bb0 wordp=0x0bb0 params=1 returns=1
// ================================================
// 0x0bb0: pop    bx
// 0x0bb1: push   word ptr [bx]
// 0x0bb3: lodsw
// 0x0bb4: mov    bx,ax
// 0x0bb6: jmp    word ptr [bx]

// ================================================
// 0x0bb8: WORD '@L' codep=0x0bbf wordp=0x0bbf params=2 returns=1
// ================================================
// 0x0bbf: pop    ax
// 0x0bc0: mov    ah,al
// 0x0bc2: sub    al,al
// 0x0bc4: add    ah,ah
// 0x0bc6: add    ah,ah
// 0x0bc8: add    ah,ah
// 0x0bca: add    ah,ah
// 0x0bcc: mov    ds,ax
// 0x0bce: pop    bx
// 0x0bcf: mov    ax,[bx]
// 0x0bd1: push   ax
// 0x0bd2: mov    ax,cs
// 0x0bd4: mov    ds,ax
// 0x0bd6: lodsw
// 0x0bd7: mov    bx,ax
// 0x0bd9: jmp    word ptr [bx]

// ================================================
// 0x0bdb: WORD '!' codep=0x0be1 wordp=0x0be1 params=2 returns=0
// ================================================
// 0x0be1: pop    bx
// 0x0be2: pop    ax
// 0x0be3: mov    [bx],ax
// 0x0be5: lodsw
// 0x0be6: mov    bx,ax
// 0x0be8: jmp    word ptr [bx]

// ================================================
// 0x0bea: WORD '!L' codep=0x0bf1 wordp=0x0bf1 params=3 returns=0
// ================================================
// 0x0bf1: pop    ax
// 0x0bf2: mov    ah,al
// 0x0bf4: sub    al,al
// 0x0bf6: add    ah,ah
// 0x0bf8: add    ah,ah
// 0x0bfa: add    ah,ah
// 0x0bfc: add    ah,ah
// 0x0bfe: mov    ds,ax
// 0x0c00: pop    bx
// 0x0c01: pop    ax
// 0x0c02: mov    [bx],ax
// 0x0c04: mov    ax,cs
// 0x0c06: mov    ds,ax
// 0x0c08: lodsw
// 0x0c09: mov    bx,ax
// 0x0c0b: jmp    word ptr [bx]

// ================================================
// 0x0c0d: WORD '(CS?)' codep=0x0c17 wordp=0x0c17 params=0 returns=1
// ================================================
// 0x0c17: push   cs
// 0x0c18: lodsw
// 0x0c19: mov    bx,ax
// 0x0c1b: jmp    word ptr [bx]

// ================================================
// 0x0c1d: WORD '2!_1' codep=0x0c24 wordp=0x0c24
// ================================================
// 0x0c24: pop    bx
// 0x0c25: pop    ax
// 0x0c26: pop    cx
// 0x0c27: cli    
// 0x0c28: mov    [bx],ax
// 0x0c2a: mov    [bx+02],cx
// 0x0c2d: sti    
// 0x0c2e: lodsw
// 0x0c2f: mov    bx,ax
// 0x0c31: jmp    word ptr [bx]

// ================================================
// 0x0c33: WORD '2@' codep=0x0c3a wordp=0x0c3a params=1 returns=2
// ================================================
// 0x0c3a: pop    bx
// 0x0c3b: cli    
// 0x0c3c: mov    ax,[bx]
// 0x0c3e: mov    dx,[bx+02]
// 0x0c41: sti    
// 0x0c42: push   dx
// 0x0c43: push   ax
// 0x0c44: lodsw
// 0x0c45: mov    bx,ax
// 0x0c47: jmp    word ptr [bx]

// ================================================
// 0x0c49: WORD 'BLANK' codep=0x224c wordp=0x0c53
// ================================================

void BLANK() // BLANK
{
  Push(Read16(cc_BL)); // BL
  FILL_1(); // FILL_1
}


// ================================================
// 0x0c59: WORD 'C!_1' codep=0x0c60 wordp=0x0c60
// ================================================
// 0x0c60: pop    bx
// 0x0c61: pop    ax
// 0x0c62: mov    [bx],al
// 0x0c64: lodsw
// 0x0c65: mov    bx,ax
// 0x0c67: jmp    word ptr [bx]

// ================================================
// 0x0c69: WORD 'C!L' codep=0x0c71 wordp=0x0c71 params=3 returns=0
// ================================================
// 0x0c71: pop    ax
// 0x0c72: mov    ah,al
// 0x0c74: sub    al,al
// 0x0c76: add    ah,ah
// 0x0c78: add    ah,ah
// 0x0c7a: add    ah,ah
// 0x0c7c: add    ah,ah
// 0x0c7e: mov    ds,ax
// 0x0c80: pop    bx
// 0x0c81: pop    ax
// 0x0c82: mov    [bx],al
// 0x0c84: mov    ax,cs
// 0x0c86: mov    ds,ax
// 0x0c88: lodsw
// 0x0c89: mov    bx,ax
// 0x0c8b: jmp    word ptr [bx]

// ================================================
// 0x0c8d: WORD 'C@' codep=0x0c94 wordp=0x0c94 params=1 returns=1
// ================================================
// 0x0c94: pop    bx
// 0x0c95: sub    ax,ax
// 0x0c97: mov    al,[bx]
// 0x0c99: push   ax
// 0x0c9a: lodsw
// 0x0c9b: mov    bx,ax
// 0x0c9d: jmp    word ptr [bx]

// ================================================
// 0x0c9f: WORD 'C@L' codep=0x0ca7 wordp=0x0ca7 params=2 returns=1
// ================================================
// 0x0ca7: pop    ax
// 0x0ca8: mov    ah,al
// 0x0caa: sub    al,al
// 0x0cac: add    ah,ah
// 0x0cae: add    ah,ah
// 0x0cb0: add    ah,ah
// 0x0cb2: add    ah,ah
// 0x0cb4: mov    ds,ax
// 0x0cb6: pop    bx
// 0x0cb7: mov    al,[bx]
// 0x0cb9: sub    ah,ah
// 0x0cbb: push   ax
// 0x0cbc: mov    ax,cs
// 0x0cbe: mov    ds,ax
// 0x0cc0: lodsw
// 0x0cc1: mov    bx,ax
// 0x0cc3: jmp    word ptr [bx]

// ================================================
// 0x0cc5: WORD 'COUNT' codep=0x224c wordp=0x0ccf params=1 returns=2
// ================================================

void COUNT() // COUNT
{
  Push(Read16(regsp)); // DUP
  Push(Pop() + 1); //  1+
  SWAP(); // SWAP
  Push(Read16(Pop())&0xFF); //  C@
}


// ================================================
// 0x0cd9: WORD 'CMOVE>' codep=0x0ce4 wordp=0x0ce4 params=3 returns=0
// ================================================
// 0x0ce4: mov    ax,ds
// 0x0ce6: mov    es,ax
// 0x0ce8: mov    dx,di
// 0x0cea: mov    bx,si
// 0x0cec: pop    cx
// 0x0ced: pop    di
// 0x0cee: pop    si
// 0x0cef: or     cx,cx
// 0x0cf1: jz     0CFD
// 0x0cf3: std    
// 0x0cf4: add    di,cx
// 0x0cf6: dec    di
// 0x0cf7: add    si,cx
// 0x0cf9: dec    si
// 0x0cfa: repz   
// 0x0cfb: movsb
// 0x0cfc: cld    
// 0x0cfd: mov    di,dx
// 0x0cff: mov    si,bx
// 0x0d01: lodsw
// 0x0d02: mov    bx,ax
// 0x0d04: jmp    word ptr [bx]

// ================================================
// 0x0d06: WORD 'CMOVE_1' codep=0x0d10 wordp=0x0d10 params=3 returns=0
// ================================================
// 0x0d10: mov    ax,ds
// 0x0d12: mov    es,ax
// 0x0d14: pop    cx
// 0x0d15: mov    dx,di
// 0x0d17: pop    di
// 0x0d18: mov    bx,si
// 0x0d1a: pop    si
// 0x0d1b: clc    
// 0x0d1c: rcr    cx,1
// 0x0d1e: jae    0D21
// 0x0d20: movsb
// 0x0d21: repz   
// 0x0d22: movsw
// 0x0d23: mov    si,bx
// 0x0d25: mov    di,dx
// 0x0d27: lodsw
// 0x0d28: mov    bx,ax
// 0x0d2a: jmp    word ptr [bx]

// ================================================
// 0x0d2c: WORD 'FILL_1' codep=0x0d35 wordp=0x0d35
// ================================================
// 0x0d35: mov    ax,ds
// 0x0d37: mov    es,ax
// 0x0d39: pop    ax
// 0x0d3a: mov    ah,al
// 0x0d3c: pop    cx
// 0x0d3d: mov    dx,di
// 0x0d3f: pop    di
// 0x0d40: clc    
// 0x0d41: rcr    cx,1
// 0x0d43: jae    0D46
// 0x0d45: stosb
// 0x0d46: repz   
// 0x0d47: stosw
// 0x0d48: mov    di,dx
// 0x0d4a: lodsw
// 0x0d4b: mov    bx,ax
// 0x0d4d: jmp    word ptr [bx]

// ================================================
// 0x0d4f: WORD 'P!' codep=0x0d56 wordp=0x0d56 params=2 returns=0
// ================================================
// 0x0d56: pop    dx
// 0x0d57: pop    ax
// 0x0d58: out    dx,al
// 0x0d59: lodsw
// 0x0d5a: mov    bx,ax
// 0x0d5c: jmp    word ptr [bx]

// ================================================
// 0x0d5e: WORD 'P@' codep=0x0d65 wordp=0x0d65 params=1 returns=1
// ================================================
// 0x0d65: sub    ax,ax
// 0x0d67: pop    dx
// 0x0d68: in     al,dx
// 0x0d69: push   ax
// 0x0d6a: lodsw
// 0x0d6b: mov    bx,ax
// 0x0d6d: jmp    word ptr [bx]

// ================================================
// 0x0d6f: WORD 'SEG>ADDR' codep=0x0d7c wordp=0x0d7c params=1 returns=1
// ================================================
// 0x0d7c: pop    ax
// 0x0d7d: mov    bx,cs
// 0x0d7f: sub    ax,bx
// 0x0d81: shl    ax,1
// 0x0d83: shl    ax,1
// 0x0d85: shl    ax,1
// 0x0d87: shl    ax,1
// 0x0d89: push   ax
// 0x0d8a: lodsw
// 0x0d8b: mov    bx,ax
// 0x0d8d: jmp    word ptr [bx]

// ================================================
// 0x0d8f: WORD 'ADDR>SEG' codep=0x0d9c wordp=0x0d9c params=1 returns=1
// ================================================
// 0x0d9c: pop    ax
// 0x0d9d: shr    ax,1
// 0x0d9f: shr    ax,1
// 0x0da1: shr    ax,1
// 0x0da3: shr    ax,1
// 0x0da5: mov    cx,ds
// 0x0da7: add    ax,cx
// 0x0da9: push   ax
// 0x0daa: lodsw
// 0x0dab: mov    bx,ax
// 0x0dad: jmp    word ptr [bx]

// ================================================
// 0x0daf: WORD '>R' codep=0x0db6 wordp=0x0db6 params=1 returns=0
// ================================================
// 0x0db6: pop    bx
// 0x0db7: dec    bp
// 0x0db8: dec    bp
// 0x0db9: mov    [bp+00],bx
// 0x0dbc: lodsw
// 0x0dbd: mov    bx,ax
// 0x0dbf: jmp    word ptr [bx]

// ================================================
// 0x0dc1: WORD '?DUP' codep=0x0dca wordp=0x0dca params=1 returns=1
// ================================================
// 0x0dca: pop    ax
// 0x0dcb: or     ax,ax
// 0x0dcd: jz     0DD0
// 0x0dcf: push   ax
// 0x0dd0: push   ax
// 0x0dd1: lodsw
// 0x0dd2: mov    bx,ax
// 0x0dd4: jmp    word ptr [bx]

// ================================================
// 0x0dd6: WORD '2DROP' codep=0x0de0 wordp=0x0de0 params=2 returns=0
// ================================================
// 0x0de0: add    sp,0004
// 0x0de4: lodsw
// 0x0de5: mov    bx,ax
// 0x0de7: jmp    word ptr [bx]

// ================================================
// 0x0de9: WORD '2DUP' codep=0x0df2 wordp=0x0df2 params=2 returns=4
// ================================================
// 0x0df2: mov    bx,sp
// 0x0df4: push   word ptr [bx+02]
// 0x0df7: push   word ptr [bx]
// 0x0df9: lodsw
// 0x0dfa: mov    bx,ax
// 0x0dfc: jmp    word ptr [bx]

// ================================================
// 0x0dfe: WORD '2SWAP' codep=0x0e08 wordp=0x0e08 params=4 returns=4
// ================================================
// 0x0e08: pop    ax
// 0x0e09: pop    bx
// 0x0e0a: pop    cx
// 0x0e0b: pop    dx
// 0x0e0c: push   bx
// 0x0e0d: push   ax
// 0x0e0e: push   dx
// 0x0e0f: push   cx
// 0x0e10: lodsw
// 0x0e11: mov    bx,ax
// 0x0e13: jmp    word ptr [bx]

// ================================================
// 0x0e15: WORD 'DEPTH' codep=0x224c wordp=0x0e1f params=0 returns=1
// ================================================

void DEPTH() // DEPTH
{
  SP_at_(); // SP@
  S0(); // S0
  SWAP(); // SWAP
  _dash_(); // -
  Push(Pop() >> 1); //  2/
}


// ================================================
// 0x0e2b: WORD 'DROP' codep=0x0e34 wordp=0x0e34 params=1 returns=0
// ================================================
// 0x0e34: inc    sp
// 0x0e35: inc    sp
// 0x0e36: lodsw
// 0x0e37: mov    bx,ax
// 0x0e39: jmp    word ptr [bx]

// ================================================
// 0x0e3b: WORD 'DUP' codep=0x0e43 wordp=0x0e43 params=1 returns=2
// ================================================
// 0x0e43: mov    bx,sp
// 0x0e45: push   word ptr [bx]
// 0x0e47: lodsw
// 0x0e48: mov    bx,ax
// 0x0e4a: jmp    word ptr [bx]

// ================================================
// 0x0e4c: WORD 'I' codep=0x0e52 wordp=0x0e52 params=0 returns=1
// ================================================
// 0x0e52: mov    ax,[bp+00]
// 0x0e55: push   ax
// 0x0e56: lodsw
// 0x0e57: mov    bx,ax
// 0x0e59: jmp    word ptr [bx]

// ================================================
// 0x0e5b: WORD 'I'' codep=0x0e62 wordp=0x0e62 params=0 returns=1
// ================================================
// 0x0e62: push   word ptr [bp+02]
// 0x0e65: lodsw
// 0x0e66: mov    bx,ax
// 0x0e68: jmp    word ptr [bx]

// ================================================
// 0x0e6a: WORD 'J' codep=0x0e70 wordp=0x0e70 params=0 returns=1
// ================================================
// 0x0e70: push   word ptr [bp+04]
// 0x0e73: lodsw
// 0x0e74: mov    bx,ax
// 0x0e76: jmp    word ptr [bx]

// ================================================
// 0x0e78: WORD 'OVER' codep=0x0e81 wordp=0x0e81 params=2 returns=3
// ================================================
// 0x0e81: mov    bx,sp
// 0x0e83: push   word ptr [bx+02]
// 0x0e86: lodsw
// 0x0e87: mov    bx,ax
// 0x0e89: jmp    word ptr [bx]

// ================================================
// 0x0e8b: WORD 'R>' codep=0x0e92 wordp=0x0e92 params=0 returns=1
// ================================================
// 0x0e92: mov    ax,[bp+00]
// 0x0e95: inc    bp
// 0x0e96: inc    bp
// 0x0e97: push   ax
// 0x0e98: lodsw
// 0x0e99: mov    bx,ax
// 0x0e9b: jmp    word ptr [bx]

// ================================================
// 0x0e9d: WORD 'R@' codep=0x0ea4 wordp=0x0ea4 params=0 returns=1
// ================================================
// 0x0ea4: mov    ax,[bp+00]
// 0x0ea7: push   ax
// 0x0ea8: lodsw
// 0x0ea9: mov    bx,ax
// 0x0eab: jmp    word ptr [bx]

// ================================================
// 0x0ead: WORD 'ROT' codep=0x0eb5 wordp=0x0eb5 params=3 returns=3
// ================================================
// 0x0eb5: pop    dx
// 0x0eb6: pop    bx
// 0x0eb7: pop    ax
// 0x0eb8: push   bx
// 0x0eb9: push   dx
// 0x0eba: push   ax
// 0x0ebb: lodsw
// 0x0ebc: mov    bx,ax
// 0x0ebe: jmp    word ptr [bx]

// ================================================
// 0x0ec0: WORD 'RP!' codep=0x0ec8 wordp=0x0ec8 params=0 returns=0
// ================================================
// 0x0ec8: mov    bp,[di+02]
// 0x0ecb: lodsw
// 0x0ecc: mov    bx,ax
// 0x0ece: jmp    word ptr [bx]

// ================================================
// 0x0ed0: WORD 'RP@' codep=0x0ed8 wordp=0x0ed8 params=0 returns=1
// ================================================
// 0x0ed8: push   bp
// 0x0ed9: lodsw
// 0x0eda: mov    bx,ax
// 0x0edc: jmp    word ptr [bx]

// ================================================
// 0x0ede: WORD 'S0' codep=0x224c wordp=0x0ee5 params=0 returns=1
// ================================================

void S0() // S0
{
  Push(Read16(user_SP0)); // SP0 @
}


// ================================================
// 0x0eeb: WORD 'SWAP' codep=0x0ef4 wordp=0x0ef4 params=2 returns=2
// ================================================
// 0x0ef4: pop    dx
// 0x0ef5: pop    ax
// 0x0ef6: push   dx
// 0x0ef7: push   ax
// 0x0ef8: lodsw
// 0x0ef9: mov    bx,ax
// 0x0efb: jmp    word ptr [bx]

// ================================================
// 0x0efd: WORD 'SP!' codep=0x0f05 wordp=0x0f05 params=0 returns=0
// ================================================
// 0x0f05: mov    sp,[di]
// 0x0f07: lodsw
// 0x0f08: mov    bx,ax
// 0x0f0a: jmp    word ptr [bx]

// ================================================
// 0x0f0c: WORD 'SP@' codep=0x0f14 wordp=0x0f14 params=0 returns=1
// ================================================
// 0x0f14: mov    ax,sp
// 0x0f16: push   ax
// 0x0f17: lodsw
// 0x0f18: mov    bx,ax
// 0x0f1a: jmp    word ptr [bx]

// ================================================
// 0x0f1c: WORD '0' codep=0x0f22 wordp=0x0f22 params=0 returns=1
// ================================================
// 0x0f22: sub    ax,ax
// 0x0f24: push   ax
// 0x0f25: lodsw
// 0x0f26: mov    bx,ax
// 0x0f28: jmp    word ptr [bx]

// ================================================
// 0x0f2a: WORD '1' codep=0x0f30 wordp=0x0f30 params=0 returns=1
// ================================================
// 0x0f30: mov    ax,0001
// 0x0f33: push   ax
// 0x0f34: lodsw
// 0x0f35: mov    bx,ax
// 0x0f37: jmp    word ptr [bx]

// ================================================
// 0x0f39: WORD '2' codep=0x0f3f wordp=0x0f3f params=0 returns=1
// ================================================
// 0x0f3f: mov    ax,0002
// 0x0f42: push   ax
// 0x0f43: lodsw
// 0x0f44: mov    bx,ax
// 0x0f46: jmp    word ptr [bx]

// ================================================
// 0x0f48: WORD '/' codep=0x0f4e wordp=0x0f4e params=2 returns=2
// ================================================
// 0x0f4e: pop    bx
// 0x0f4f: pop    ax
// 0x0f50: cwd    
// 0x0f51: idiv   bx
// 0x0f53: push   ax
// 0x0f54: lodsw
// 0x0f55: mov    bx,ax
// 0x0f57: jmp    word ptr [bx]

// ================================================
// 0x0f59: WORD '/MOD' codep=0x0f62 wordp=0x0f62 params=2 returns=2
// ================================================
// 0x0f62: pop    bx
// 0x0f63: pop    ax
// 0x0f64: cwd    
// 0x0f65: idiv   bx
// 0x0f67: push   dx
// 0x0f68: push   ax
// 0x0f69: lodsw
// 0x0f6a: mov    bx,ax
// 0x0f6c: jmp    word ptr [bx]

// ================================================
// 0x0f6e: WORD '+' codep=0x0f74 wordp=0x0f74 params=2 returns=1
// ================================================
// 0x0f74: pop    ax
// 0x0f75: pop    bx
// 0x0f76: add    ax,bx
// 0x0f78: push   ax
// 0x0f79: lodsw
// 0x0f7a: mov    bx,ax
// 0x0f7c: jmp    word ptr [bx]

// ================================================
// 0x0f7e: WORD '+!' codep=0x0f85 wordp=0x0f85 params=2 returns=0
// ================================================
// 0x0f85: pop    bx
// 0x0f86: pop    ax
// 0x0f87: add    [bx],ax
// 0x0f89: lodsw
// 0x0f8a: mov    bx,ax
// 0x0f8c: jmp    word ptr [bx]

// ================================================
// 0x0f8e: WORD '-' codep=0x0f94 wordp=0x0f94 params=2 returns=1
// ================================================
// 0x0f94: pop    bx
// 0x0f95: pop    ax
// 0x0f96: sub    ax,bx
// 0x0f98: push   ax
// 0x0f99: lodsw
// 0x0f9a: mov    bx,ax
// 0x0f9c: jmp    word ptr [bx]

// ================================================
// 0x0f9e: WORD '+-' codep=0x224c wordp=0x0fa5 params=1 returns=0
// ================================================

void _plus__dash_() // +-
{
  _0_st_(); // 0<
  if (Pop() == 0) return;
  Push(-Pop()); //  NEGATE
}


// ================================================
// 0x0faf: WORD '*' codep=0x0fb5 wordp=0x0fb5 params=2 returns=1
// ================================================
// 0x0fb5: pop    ax
// 0x0fb6: pop    bx
// 0x0fb7: mul    bx
// 0x0fb9: push   ax
// 0x0fba: lodsw
// 0x0fbb: mov    bx,ax
// 0x0fbd: jmp    word ptr [bx]

// ================================================
// 0x0fbf: WORD '*/' codep=0x224c wordp=0x0fc6 params=3 returns=1
// ================================================

void _star__slash_() // */
{
  _star__slash_MOD(); // */MOD
  SWAP(); // SWAP
  Pop(); // DROP
}


// ================================================
// 0x0fce: WORD '*/MOD' codep=0x224c wordp=0x0fd8 params=3 returns=2
// ================================================

void _star__slash_MOD() // */MOD
{
  unsigned short int a;
  a = Pop(); // >R
  M_star_(); // M*
  Push(a); // R>
  M_slash_(); // M/
}


// ================================================
// 0x0fe2: WORD '1+' codep=0x0fe9 wordp=0x0fe9 params=1 returns=1
// ================================================
// 0x0fe9: pop    ax
// 0x0fea: inc    ax
// 0x0feb: push   ax
// 0x0fec: lodsw
// 0x0fed: mov    bx,ax
// 0x0fef: jmp    word ptr [bx]

// ================================================
// 0x0ff1: WORD '1-' codep=0x0ff8 wordp=0x0ff8 params=1 returns=1
// ================================================
// 0x0ff8: pop    ax
// 0x0ff9: dec    ax
// 0x0ffa: push   ax
// 0x0ffb: lodsw
// 0x0ffc: mov    bx,ax
// 0x0ffe: jmp    word ptr [bx]

// ================================================
// 0x1000: WORD '2*' codep=0x1007 wordp=0x1007 params=1 returns=1
// ================================================
// 0x1007: pop    ax
// 0x1008: add    ax,ax
// 0x100a: push   ax
// 0x100b: lodsw
// 0x100c: mov    bx,ax
// 0x100e: jmp    word ptr [bx]

// ================================================
// 0x1010: WORD '2+' codep=0x1017 wordp=0x1017 params=1 returns=1
// ================================================
// 0x1017: pop    ax
// 0x1018: inc    ax
// 0x1019: inc    ax
// 0x101a: push   ax
// 0x101b: lodsw
// 0x101c: mov    bx,ax
// 0x101e: jmp    word ptr [bx]

// ================================================
// 0x1020: WORD '2-' codep=0x1027 wordp=0x1027 params=1 returns=1
// ================================================
// 0x1027: pop    ax
// 0x1028: dec    ax
// 0x1029: dec    ax
// 0x102a: push   ax
// 0x102b: lodsw
// 0x102c: mov    bx,ax
// 0x102e: jmp    word ptr [bx]

// ================================================
// 0x1030: WORD '2/' codep=0x1037 wordp=0x1037 params=1 returns=1
// ================================================
// 0x1037: pop    ax
// 0x1038: sar    ax,1
// 0x103a: push   ax
// 0x103b: lodsw
// 0x103c: mov    bx,ax
// 0x103e: jmp    word ptr [bx]

// ================================================
// 0x1040: WORD '4*' codep=0x1047 wordp=0x1047 params=1 returns=1
// ================================================
// 0x1047: pop    ax
// 0x1048: shl    ax,1
// 0x104a: shl    ax,1
// 0x104c: push   ax
// 0x104d: lodsw
// 0x104e: mov    bx,ax
// 0x1050: jmp    word ptr [bx]

// ================================================
// 0x1052: WORD 'ABS' codep=0x224c wordp=0x105a params=1 returns=1
// ================================================

void ABS() // ABS
{
  Push(Read16(regsp)); // DUP
  _plus__dash_(); // +-
}


// ================================================
// 0x1060: WORD 'D+' codep=0x1067 wordp=0x1067 params=4 returns=2
// ================================================
// 0x1067: pop    ax
// 0x1068: pop    dx
// 0x1069: pop    bx
// 0x106a: pop    cx
// 0x106b: add    dx,cx
// 0x106d: adc    ax,bx
// 0x106f: push   dx
// 0x1070: push   ax
// 0x1071: lodsw
// 0x1072: mov    bx,ax
// 0x1074: jmp    word ptr [bx]

// ================================================
// 0x1076: WORD 'D+-' codep=0x224c wordp=0x107e params=1 returns=0
// ================================================

void D_plus__dash_() // D+-
{
  _0_st_(); // 0<
  if (Pop() == 0) return;
  DNEGATE(); // DNEGATE
}


// ================================================
// 0x1088: WORD 'DECIMAL' codep=0x224c wordp=0x1094 params=0 returns=0
// ================================================

void DECIMAL() // DECIMAL
{
  Push(0x000a);
  Push(user_BASE); // BASE
  Store(); // !
}


// ================================================
// 0x109e: WORD 'DABS' codep=0x224c wordp=0x10a7 params=1 returns=1
// ================================================

void DABS() // DABS
{
  Push(Read16(regsp)); // DUP
  D_plus__dash_(); // D+-
}


// ================================================
// 0x10ad: WORD 'DNEGATE' codep=0x10b9 wordp=0x10b9 params=2 returns=2
// ================================================
// 0x10b9: sub    ax,ax
// 0x10bb: pop    cx
// 0x10bc: pop    dx
// 0x10bd: neg    dx
// 0x10bf: sbb    ax,cx
// 0x10c1: push   dx
// 0x10c2: push   ax
// 0x10c3: lodsw
// 0x10c4: mov    bx,ax
// 0x10c6: jmp    word ptr [bx]

// ================================================
// 0x10c8: WORD 'M*' codep=0x224c wordp=0x10cf params=2 returns=2
// ================================================

void M_star_() // M*
{
  unsigned short int a;
  _2DUP(); // 2DUP
  Push(Pop() ^ Pop()); // XOR
  a = Pop(); // >R
  ABS(); // ABS
  SWAP(); // SWAP
  ABS(); // ABS
  U_star_(); // U*
  Push(a); // R>
  D_plus__dash_(); // D+-
}


// ================================================
// 0x10e3: WORD 'M*/' codep=0x224c wordp=0x10eb params=4 returns=2
// ================================================

void M_star__slash_() // M*/
{
  unsigned short int a, b;
  _2DUP(); // 2DUP
  Push(Pop() ^ Pop()); // XOR
  SWAP(); // SWAP
  ABS(); // ABS
  a = Pop(); // >R
  SWAP(); // SWAP
  ABS(); // ABS
  b = Pop(); // >R
  OVER(); // OVER
  Push(Pop() ^ Pop()); // XOR
  ROT(); // ROT
  ROT(); // ROT
  DABS(); // DABS
  SWAP(); // SWAP
  Push(Read16(b)); // R@
  U_star_(); // U*
  ROT(); // ROT
  Push(b); // R>
  U_star_(); // U*
  ROT(); // ROT
  Push(0);
  D_plus_(); // D+
  Push(Read16(a)); // R@
  U_slash_MOD(); // U/MOD
  ROT(); // ROT
  ROT(); // ROT
  Push(a); // R>
  U_slash_MOD(); // U/MOD
  SWAP(); // SWAP
  Pop(); // DROP
  SWAP(); // SWAP
  ROT(); // ROT
  D_plus__dash_(); // D+-
}


// ================================================
// 0x112f: WORD 'M+' codep=0x224c wordp=0x1136 params=3 returns=2
// ================================================

void M_plus_() // M+
{
  S_dash__gt_D(); // S->D
  D_plus_(); // D+
}


// ================================================
// 0x113c: WORD 'M/' codep=0x224c wordp=0x1143 params=3 returns=2
// ================================================

void M_slash_() // M/
{
  unsigned short int a, b;
  OVER(); // OVER
  a = Pop(); // >R
  b = Pop(); // >R
  Push(Read16(regsp)); // DUP
  D_plus__dash_(); // D+-
  Push(Read16(b)); // R@
  ABS(); // ABS
  U_slash_MOD(); // U/MOD
  Push(b); // R>
  Push(Read16(a)); // R@
  Push(Pop() ^ Pop()); // XOR
  _plus__dash_(); // +-
  SWAP(); // SWAP
  Push(a); // R>
  _plus__dash_(); // +-
  SWAP(); // SWAP
}


// ================================================
// 0x1165: WORD 'M/MOD' codep=0x224c wordp=0x116f params=3 returns=3
// ================================================

void M_slash_MOD() // M/MOD
{
  unsigned short int a, b;
  a = Pop(); // >R
  Push(0);
  Push(Read16(a)); // R@
  U_slash_MOD(); // U/MOD
  Push(a); // R>
  SWAP(); // SWAP
  b = Pop(); // >R
  U_slash_MOD(); // U/MOD
  Push(b); // R>
}


// ================================================
// 0x1183: WORD 'MAX' codep=0x224c wordp=0x118b params=2 returns=1
// ================================================

void MAX() // MAX
{
  _2DUP(); // 2DUP
  _st_(); // <
  if (Pop() != 0)
  {
    SWAP(); // SWAP
  }
  Pop(); // DROP
}


// ================================================
// 0x1199: WORD 'MOD' codep=0x224c wordp=0x11a1 params=2 returns=1
// ================================================

void MOD() // MOD
{
  _slash_MOD(); // /MOD
  Pop(); // DROP
}


// ================================================
// 0x11a7: WORD 'MIN' codep=0x224c wordp=0x11af params=2 returns=1
// ================================================

void MIN() // MIN
{
  _2DUP(); // 2DUP
  _gt_(); // >
  if (Pop() != 0)
  {
    SWAP(); // SWAP
  }
  Pop(); // DROP
}


// ================================================
// 0x11bd: WORD 'NEGATE' codep=0x11c8 wordp=0x11c8 params=1 returns=1
// ================================================
// 0x11c8: pop    ax
// 0x11c9: neg    ax
// 0x11cb: push   ax
// 0x11cc: lodsw
// 0x11cd: mov    bx,ax
// 0x11cf: jmp    word ptr [bx]

// ================================================
// 0x11d1: WORD 'U*' codep=0x11d8 wordp=0x11d8 params=2 returns=2
// ================================================
// 0x11d8: pop    ax
// 0x11d9: pop    bx
// 0x11da: mul    bx
// 0x11dc: push   ax
// 0x11dd: push   dx
// 0x11de: lodsw
// 0x11df: mov    bx,ax
// 0x11e1: jmp    word ptr [bx]

// ================================================
// 0x11e3: WORD 'U/MOD' codep=0x11ed wordp=0x11ed params=3 returns=2
// ================================================
// 0x11ed: pop    bx
// 0x11ee: pop    dx
// 0x11ef: pop    ax
// 0x11f0: div    bx
// 0x11f2: push   dx
// 0x11f3: push   ax
// 0x11f4: lodsw
// 0x11f5: mov    bx,ax
// 0x11f7: jmp    word ptr [bx]

// ================================================
// 0x11f9: WORD 'UMAX' codep=0x224c wordp=0x1202 params=2 returns=1
// ================================================

void UMAX() // UMAX
{
  _2DUP(); // 2DUP
  U_st_(); // U<
  if (Pop() != 0)
  {
    SWAP(); // SWAP
  }
  Pop(); // DROP
}


// ================================================
// 0x1210: WORD 'UMIN' codep=0x224c wordp=0x1219 params=2 returns=1
// ================================================

void UMIN() // UMIN
{
  _2DUP(); // 2DUP
  U_st_(); // U<
  Push(Pop()==0?1:0); //  0=
  if (Pop() != 0)
  {
    SWAP(); // SWAP
  }
  Pop(); // DROP
}


// ================================================
// 0x1229: WORD '>' codep=0x122f wordp=0x122f params=2 returns=1
// ================================================
// 0x122f: pop    ax
// 0x1230: mov    cx,8000
// 0x1233: add    ax,cx
// 0x1235: pop    dx
// 0x1236: add    dx,cx
// 0x1238: sub    ax,dx
// 0x123a: adc    cx,cx
// 0x123c: push   cx
// 0x123d: lodsw
// 0x123e: mov    bx,ax
// 0x1240: jmp    word ptr [bx]

// ================================================
// 0x1242: WORD '<' codep=0x1248 wordp=0x1248 params=2 returns=1
// ================================================
// 0x1248: pop    ax
// 0x1249: mov    cx,8000
// 0x124c: add    ax,cx
// 0x124e: pop    dx
// 0x124f: add    dx,cx
// 0x1251: sub    dx,ax
// 0x1253: adc    cx,cx
// 0x1255: push   cx
// 0x1256: lodsw
// 0x1257: mov    bx,ax
// 0x1259: jmp    word ptr [bx]

// ================================================
// 0x125b: WORD '=' codep=0x1261 wordp=0x1261 params=2 returns=1
// ================================================
// 0x1261: pop    ax
// 0x1262: pop    dx
// 0x1263: xor    cx,cx
// 0x1265: sub    ax,dx
// 0x1267: sub    ax,0001
// 0x126b: rcl    cx,1
// 0x126d: push   cx
// 0x126e: lodsw
// 0x126f: mov    bx,ax
// 0x1271: jmp    word ptr [bx]

// ================================================
// 0x1273: WORD '0<' codep=0x127a wordp=0x127a params=1 returns=1
// ================================================
// 0x127a: pop    ax
// 0x127b: cwd    
// 0x127c: neg    dx
// 0x127e: push   dx
// 0x127f: lodsw
// 0x1280: mov    bx,ax
// 0x1282: jmp    word ptr [bx]

// ================================================
// 0x1284: WORD '0=' codep=0x128b wordp=0x128b params=1 returns=1
// ================================================
// 0x128b: pop    ax
// 0x128c: xor    cx,cx
// 0x128e: sub    ax,0001
// 0x1292: rcl    cx,1
// 0x1294: push   cx
// 0x1295: lodsw
// 0x1296: mov    bx,ax
// 0x1298: jmp    word ptr [bx]

// ================================================
// 0x129a: WORD '0>' codep=0x12a1 wordp=0x12a1 params=1 returns=1
// ================================================
// 0x12a1: pop    ax
// 0x12a2: neg    ax
// 0x12a4: cwd    
// 0x12a5: neg    dx
// 0x12a7: push   dx
// 0x12a8: lodsw
// 0x12a9: mov    bx,ax
// 0x12ab: jmp    word ptr [bx]

// ================================================
// 0x12ad: WORD 'D<' codep=0x224c wordp=0x12b4 params=4 returns=1
// ================================================

void D_st_() // D<
{
  ROT(); // ROT
  _2DUP(); // 2DUP
  Push((Pop()==Pop())?1:0); // =
  if (Pop() != 0)
  {
    ROT(); // ROT
    ROT(); // ROT
    DNEGATE(); // DNEGATE
    D_plus_(); // D+
    _0_st_(); // 0<
  } else
  {
    SWAP(); // SWAP
    _st_(); // <
    SWAP(); // SWAP
    Pop(); // DROP
  }
  SWAP(); // SWAP
  Pop(); // DROP
}


// ================================================
// 0x12da: WORD 'U<' codep=0x12e1 wordp=0x12e1 params=2 returns=1
// ================================================
// 0x12e1: pop    ax
// 0x12e2: pop    dx
// 0x12e3: xor    cx,cx
// 0x12e5: sub    dx,ax
// 0x12e7: rcl    cx,1
// 0x12e9: push   cx
// 0x12ea: lodsw
// 0x12eb: mov    bx,ax
// 0x12ed: jmp    word ptr [bx]

// ================================================
// 0x12ef: WORD 'AND' codep=0x12f7 wordp=0x12f7 params=2 returns=1
// ================================================
// 0x12f7: pop    ax
// 0x12f8: pop    bx
// 0x12f9: and    ax,bx
// 0x12fb: push   ax
// 0x12fc: lodsw
// 0x12fd: mov    bx,ax
// 0x12ff: jmp    word ptr [bx]

// ================================================
// 0x1301: WORD 'NOT' codep=0x1309 wordp=0x1309 params=1 returns=1
// ================================================
// 0x1309: pop    ax
// 0x130a: xor    cx,cx
// 0x130c: sub    ax,0001
// 0x1310: rcl    cx,1
// 0x1312: push   cx
// 0x1313: lodsw
// 0x1314: mov    bx,ax
// 0x1316: jmp    word ptr [bx]

// ================================================
// 0x1318: WORD 'OFF_1' codep=0x224c wordp=0x1320 params=1 returns=0
// ================================================

void OFF_1() // OFF_1
{
  Push(0);
  SWAP(); // SWAP
  Store(); // !
}


// ================================================
// 0x1328: WORD 'ON_3' codep=0x224c wordp=0x132f params=1 returns=0
// ================================================

void ON_3() // ON_3
{
  Push(-1);
  SWAP(); // SWAP
  Store(); // !
}


// ================================================
// 0x1339: WORD 'OR' codep=0x1340 wordp=0x1340 params=2 returns=1
// ================================================
// 0x1340: pop    ax
// 0x1341: pop    bx
// 0x1342: or     ax,bx
// 0x1344: push   ax
// 0x1345: lodsw
// 0x1346: mov    bx,ax
// 0x1348: jmp    word ptr [bx]

// ================================================
// 0x134a: WORD 'TOGGLE' codep=0x1355 wordp=0x1355 params=2 returns=0
// ================================================
// 0x1355: pop    ax
// 0x1356: pop    bx
// 0x1357: xor    [bx],al
// 0x1359: lodsw
// 0x135a: mov    bx,ax
// 0x135c: jmp    word ptr [bx]

// ================================================
// 0x135e: WORD 'XOR' codep=0x1366 wordp=0x1366 params=2 returns=1
// ================================================
// 0x1366: pop    ax
// 0x1367: pop    bx
// 0x1368: xor    ax,bx
// 0x136a: push   ax
// 0x136b: lodsw
// 0x136c: mov    bx,ax
// 0x136e: jmp    word ptr [bx]

// ================================================
// 0x1370: WORD '#' codep=0x224c wordp=0x1376
// ================================================

void _n_() // #
{
  Push(Read16(user_BASE)); // BASE @
  M_slash_MOD(); // M/MOD
  ROT(); // ROT
  Push(9);
  OVER(); // OVER
  _st_(); // <
  if (Pop() != 0)
  {
    Push(Pop() + 7); //  7 +
  }
  Push(Pop() + 0x0030); //  0x0030 +
  HOLD(); // HOLD
}


// ================================================
// 0x139a: WORD '#>' codep=0x224c wordp=0x13a1 params=2 returns=2
// ================================================

void _n__gt_() // #>
{
  Pop(); Pop(); // 2DROP
  Push(Read16(user_HLD)); // HLD @
  PAD(); // PAD
  OVER(); // OVER
  _dash_(); // -
}


// ================================================
// 0x13af: WORD '#S' codep=0x224c wordp=0x13b6
// ================================================

void _n_S() // #S
{
  do
  {
    _n_(); // #
    _2DUP(); // 2DUP
    Push(Pop() | Pop()); // OR
    Push(!Pop()); //  NOT
  } while(Pop() == 0);
}


// ================================================
// 0x13c4: WORD '(NUMBER)' codep=0x224c wordp=0x13d1 params=1 returns=3
// ================================================

void _ro_NUMBER_rc_() // (NUMBER)
{
  unsigned short int a;
  Push(0);
  Push(0);
  ROT(); // ROT
  Push(Read16(regsp)); // DUP
  Push((Read16(Pop() + 1)&0xFF)==0x002d?1:0); //  1+ C@ 0x002d =
  Push(Read16(regsp)); // DUP
  a = Pop(); // >R
  Push(Pop() + Pop()); // +
  Push(-1);
  Push(user_DPL); // DPL
  Store(); // !
  CONVERT(); // CONVERT
  Push(Read16(regsp)); // DUP
  Push(Read16(Pop())&0xFF); //  C@
  Push(Read16(cc_BL)); // BL
  _gt_(); // >
  if (Pop() != 0)
  {
    Push(Read16(regsp)); // DUP
    Push(!((Read16(Pop())&0xFF)==0x002e?1:0)); //  C@ 0x002e = NOT
    W0B5D(); // W0B5D
    Push(0);
    Push(user_DPL); // DPL
    Store(); // !
    CONVERT(); // CONVERT
    Push(Read16(regsp)); // DUP
    Push(Read16(Pop())&0xFF); //  C@
    Push(Read16(cc_BL)); // BL
    _gt_(); // >
    W0B5D(); // W0B5D
  }
  Pop(); // DROP
  Push(a); // R>
  if (Pop() == 0) return;
  DNEGATE(); // DNEGATE
}


// ================================================
// 0x142b: WORD '>UPPERCASE' codep=0x143a wordp=0x143a params=2 returns=0
// ================================================
// 0x143a: pop    cx
// 0x143b: pop    bx
// 0x143c: jcxz   1453
// 0x143e: mov    dx,617A
// 0x1441: mov    al,[bx]
// 0x1443: cmp    al,dh
// 0x1445: jb     1450
// 0x1447: cmp    al,dl
// 0x1449: ja     1450
// 0x144b: and    al,DF
// 0x144e: mov    [bx],al
// 0x1450: inc    bx
// 0x1451: loop   1441
// 0x1453: lodsw
// 0x1454: mov    bx,ax
// 0x1456: jmp    word ptr [bx]

// ================================================
// 0x1458: WORD '<#' codep=0x224c wordp=0x145f params=0 returns=0
// ================================================

void _st__n_() // <#
{
  PAD(); // PAD
  Push(user_HLD); // HLD
  Store(); // !
}


// ================================================
// 0x1467: WORD 'CONVERT' codep=0x224c wordp=0x1473 params=1 returns=2
// ================================================

void CONVERT() // CONVERT
{
  unsigned short int a;
  while(1)
  {
    Push(Pop() + 1); //  1+
    Push(Read16(regsp)); // DUP
    a = Pop(); // >R
    Push(Read16(Pop())&0xFF); //  C@
    Push(Read16(user_BASE)); // BASE @
    DIGIT(); // DIGIT
    if (Pop() == 0) break;

    SWAP(); // SWAP
    Push(Read16(user_BASE)); // BASE @
    U_star_(); // U*
    Pop(); // DROP
    ROT(); // ROT
    Push(Read16(user_BASE)); // BASE @
    U_star_(); // U*
    D_plus_(); // D+
    Push(Read16(user_DPL) + 1); // DPL @ 1+
    if (Pop() != 0)
    {
      Push(1);
      Push(user_DPL); // DPL
      _plus__ex_(); // +!
    }
    Push(a); // R>
  }
  Push(a); // R>
}


// ================================================
// 0x14b3: WORD 'DIGIT' codep=0x14bd wordp=0x14bd params=2 returns=2
// ================================================
// 0x14bd: pop    dx
// 0x14be: pop    ax
// 0x14bf: sub    al,30
// 0x14c2: jb     14DE
// 0x14c4: cmp    al,09
// 0x14c7: jbe    14D1
// 0x14c9: sub    al,07
// 0x14cc: cmp    al,0A
// 0x14cf: jb     14DE
// 0x14d1: cmp    al,dl
// 0x14d3: jae    14DE
// 0x14d5: sub    dx,dx
// 0x14d7: mov    dl,al
// 0x14d9: mov    al,01
// 0x14db: push   dx
// 0x14dc: jz     14E0
// 0x14de: sub    ax,ax
// 0x14e0: push   ax
// 0x14e1: lodsw
// 0x14e2: mov    bx,ax
// 0x14e4: jmp    word ptr [bx]

// ================================================
// 0x14e6: WORD 'HOLD' codep=0x224c wordp=0x14ef
// ================================================

void HOLD() // HOLD
{
  Push(-1);
  Push(user_HLD); // HLD
  _plus__ex_(); // +!
  Push(Read16(user_HLD)); // HLD @
  C_ex__1(); // C!_1
}


// ================================================
// 0x14ff: WORD 'S->D' codep=0x1508 wordp=0x1508 params=1 returns=2
// ================================================
// 0x1508: mov    bx,sp
// 0x150a: mov    ax,[bx]
// 0x150c: cwd    
// 0x150d: push   dx
// 0x150e: lodsw
// 0x150f: mov    bx,ax
// 0x1511: jmp    word ptr [bx]

// ================================================
// 0x1513: WORD 'SIGN' codep=0x224c wordp=0x151c params=1 returns=0
// ================================================

void SIGN() // SIGN
{
  _0_st_(); // 0<
  if (Pop() == 0) return;
  Push(0x002d);
  HOLD(); // HOLD
}


// ================================================
// 0x152a: WORD '(>CODE)' codep=0x1536 wordp=0x1536
// ================================================
// 0x1536: lodsw
// 0x1537: mov    bx,si
// 0x1539: add    si,ax
// 0x153b: jmp    bx

// ================================================
// 0x153d: WORD '(;VIA)' codep=0x1548 wordp=0x1548
// ================================================
// 0x1548: lodsw
// 0x1549: inc    ax
// 0x154a: inc    ax
// 0x154b: mov    si,ax
// 0x154d: lodsw
// 0x154e: mov    bx,ax
// 0x1550: jmp    word ptr [bx]

// ================================================
// 0x1552: WORD '(+LOOP)' codep=0x155e wordp=0x155e params=1 returns=0
// ================================================
// 0x155e: pop    bx
// 0x155f: mov    ax,[bp+00]
// 0x1562: add    ax,bx
// 0x1564: or     bx,bx
// 0x1566: js     1577
// 0x1568: cmp    ax,[bp+02]
// 0x156b: jge    157C
// 0x156d: mov    [bp+00],ax
// 0x1570: add    si,[si]
// 0x1572: lodsw
// 0x1573: mov    bx,ax
// 0x1575: jmp    word ptr [bx]
// 0x1577: cmp    ax,[bp+02]
// 0x157a: jge    156D
// 0x157c: add    bp,0004
// 0x1580: inc    si
// 0x1581: inc    si
// 0x1582: lodsw
// 0x1583: mov    bx,ax
// 0x1585: jmp    word ptr [bx]

// ================================================
// 0x1587: WORD '(/LOOP)' codep=0x1593 wordp=0x1593 params=1 returns=0
// ================================================
// 0x1593: pop    ax
// 0x1594: add    ax,[bp+00]
// 0x1597: cmp    ax,[bp+02]
// 0x159a: jae    15A6
// 0x159c: mov    [bp+00],ax
// 0x159f: add    si,[si]
// 0x15a1: lodsw
// 0x15a2: mov    bx,ax
// 0x15a4: jmp    word ptr [bx]
// 0x15a6: add    bp,0004
// 0x15aa: inc    si
// 0x15ab: inc    si
// 0x15ac: lodsw
// 0x15ad: mov    bx,ax
// 0x15af: jmp    word ptr [bx]

// ================================================
// 0x15b1: WORD '(DO)' codep=0x15ba wordp=0x15ba params=2 returns=0
// ================================================
// 0x15ba: pop    dx
// 0x15bb: pop    ax
// 0x15bc: xchg   sp,bp
// 0x15be: push   ax
// 0x15bf: push   dx
// 0x15c0: xchg   sp,bp
// 0x15c2: lodsw
// 0x15c3: mov    bx,ax
// 0x15c5: jmp    word ptr [bx]

// ================================================
// 0x15c7: WORD '(LOOP)' codep=0x15d2 wordp=0x15d2 params=0 returns=0
// ================================================
// 0x15d2: mov    ax,[bp+00]
// 0x15d5: inc    ax
// 0x15d6: cmp    ax,[bp+02]
// 0x15d9: jge    15E5
// 0x15db: mov    [bp+00],ax
// 0x15de: add    si,[si]
// 0x15e0: lodsw
// 0x15e1: mov    bx,ax
// 0x15e3: jmp    word ptr [bx]
// 0x15e5: add    bp,0004
// 0x15e9: inc    si
// 0x15ea: inc    si
// 0x15eb: lodsw
// 0x15ec: mov    bx,ax
// 0x15ee: jmp    word ptr [bx]

// ================================================
// 0x15f0: WORD '0BRANCH' codep=0x15fc wordp=0x15fc params=1 returns=0
// ================================================
// 0x15fc: pop    ax
// 0x15fd: or     ax,ax
// 0x15ff: jz     1608
// 0x1601: inc    si
// 0x1602: inc    si
// 0x1603: lodsw
// 0x1604: mov    bx,ax
// 0x1606: jmp    word ptr [bx]
// 0x1608: add    si,[si]
// 0x160a: lodsw
// 0x160b: mov    bx,ax
// 0x160d: jmp    word ptr [bx]

// ================================================
// 0x160f: WORD '2LIT' codep=0x1618 wordp=0x1618 params=0 returns=2
// ================================================
// 0x1618: lodsw
// 0x1619: push   ax
// 0x161a: lodsw
// 0x161b: push   ax
// 0x161c: lodsw
// 0x161d: mov    bx,ax
// 0x161f: jmp    word ptr [bx]

// ================================================
// 0x1621: WORD 'ABORT-IT' codep=0x224c wordp=0x162e params=0 returns=0
// ================================================

void ABORT_dash_IT() // ABORT-IT
{
  Push(1);
  ABORT("Aborting execution! ", 20);// (ABORT")
}

// 0x1649: inc    bx
// 0x164a: inc    bx
// 0x164b: dec    bp
// 0x164c: dec    bp
// 0x164d: mov    [bp+00],si
// 0x1650: pop    si
// 0x1651: push   bx
// 0x1652: lodsw
// 0x1653: mov    bx,ax
// 0x1655: jmp    word ptr [bx]

// ================================================
// 0x1657: WORD 'BRANCH' codep=0x1662 wordp=0x1662 params=0 returns=0
// ================================================
// 0x1662: add    si,[si]
// 0x1664: lodsw
// 0x1665: mov    bx,ax
// 0x1667: jmp    word ptr [bx]

// ================================================
// 0x1669: WORD 'CFAEXEC' codep=0x1675 wordp=0x1675
// ================================================
// 0x1675: pop    bx
// 0x1676: jmp    word ptr [bx]

// ================================================
// 0x1678: WORD 'EXECUTE' codep=0x1684 wordp=0x1684 params=1 returns=0
// ================================================
// 0x1684: pop    bx
// 0x1685: dec    bx
// 0x1686: dec    bx
// 0x1687: jmp    word ptr [bx]

// ================================================
// 0x1689: WORD 'EXIT' codep=0x1692 wordp=0x1692 params=0 returns=0
// ================================================
// 0x1692: mov    si,[bp+00]
// 0x1695: inc    bp
// 0x1696: inc    bp
// 0x1697: lodsw
// 0x1698: mov    bx,ax
// 0x169a: jmp    word ptr [bx]

// ================================================
// 0x169c: WORD 'GO' codep=0x16a3 wordp=0x16a3 params=1 returns=0
// ================================================
// 0x16a3: pop    bx
// 0x16a4: jmp    bx

// ================================================
// 0x16a6: WORD 'REGISTERS' codep=0x1d29 wordp=0x16b4
// ================================================
// 0x16b4: db 0x02 0x02 0x00 0x15 0x56 0x48 0xc6 0x07 0xca 0x2d 0x38 0xd3 0xca 0x2d 0xf1 0xd5 0x63 0x33 0x63 0x33 '    VH   -8  -  c3c3'
// 0x16c8: int    21
// 0x16ca: ret    


// ================================================
// 0x16cb: WORD 'INTERRUPT' codep=0x16d9 wordp=0x16d9 params=1 returns=0
// ================================================
// 0x16d9: pop    ax
// 0x16da: mov    [16C9],al
// 0x16dd: push   bp
// 0x16de: push   di
// 0x16df: push   si
// 0x16e0: push   es
// 0x16e1: pushf  
// 0x16e2: mov    ax,[16B4] // REGISTERS
// 0x16e5: push   ax
// 0x16e6: popf   
// 0x16e7: mov    bx,[16B8]
// 0x16eb: mov    cx,[16BA]
// 0x16ef: mov    dx,[16BC]
// 0x16f3: mov    si,[16C2]
// 0x16f7: mov    di,[16C0]
// 0x16fb: mov    ax,[16C6]
// 0x16fe: mov    es,ax
// 0x1700: mov    ax,[16C4]
// 0x1703: mov    ds,ax
// 0x1705: cs:    
// 0x1706: mov    ax,[16B6] // REGISTERS
// 0x1709: call   16C8
// 0x170c: cs:    
// 0x170d: mov    [16B6],ax // REGISTERS
// 0x1710: mov    ax,ds
// 0x1712: cs:    
// 0x1713: mov    [16C4],ax
// 0x1716: mov    ax,cs
// 0x1718: mov    ds,ax
// 0x171a: pushf  
// 0x171b: pop    ax
// 0x171c: mov    [16B4],ax // REGISTERS
// 0x171f: mov    ax,bx
// 0x1721: mov    [16B8],ax
// 0x1724: mov    ax,cx
// 0x1726: mov    [16BA],ax
// 0x1729: mov    ax,dx
// 0x172b: mov    [16BC],ax
// 0x172e: mov    ax,di
// 0x1730: mov    [16C0],ax
// 0x1733: mov    ax,es
// 0x1735: mov    [16C6],ax
// 0x1738: popf   
// 0x1739: pop    es
// 0x173a: pop    si
// 0x173b: pop    di
// 0x173c: pop    bp
// 0x173d: lodsw
// 0x173e: mov    bx,ax
// 0x1740: jmp    word ptr [bx]

// ================================================
// 0x1742: WORD 'LEAVE' codep=0x174c wordp=0x174c
// ================================================
// 0x174c: mov    ax,[bp+00]
// 0x174f: mov    [bp+02],ax
// 0x1752: lodsw
// 0x1753: mov    bx,ax
// 0x1755: jmp    word ptr [bx]

// ================================================
// 0x1757: WORD 'LIT' codep=0x175f wordp=0x175f params=0 returns=1
// ================================================
// 0x175f: lodsw
// 0x1760: push   ax
// 0x1761: lodsw
// 0x1762: mov    bx,ax
// 0x1764: jmp    word ptr [bx]

// ================================================
// 0x1766: WORD 'SYSCALL' codep=0x1772 wordp=0x1772
// ================================================
// 0x1772: pop    dx
// 0x1773: pop    cx
// 0x1774: mov    ch,00
// 0x1776: call   0005
// 0x1779: mov    ah,00
// 0x177b: push   ax
// 0x177c: lodsw
// 0x177d: mov    bx,ax
// 0x177f: jmp    word ptr [bx]

// ================================================
// 0x1781: WORD 'USER' codep=0x224c wordp=0x178a
// ================================================

void USER() // USER
{
  OVER(); // OVER
  CONSTANT(); // CONSTANT
  Push(Pop() + Pop()); // +
  CODE(); // (;CODE) inlined assembler code
// 0x1792: inc    bx
// 0x1793: inc    bx
// 0x1794: mov    ax,[bx]
// 0x1796: add    ax,di
// 0x1798: push   ax
// 0x1799: lodsw
// 0x179a: mov    bx,ax
// 0x179c: jmp    word ptr [bx]
}


// ================================================
// 0x179e: WORD 'USEREXECUTOR' codep=0x224c wordp=0x17af
// ================================================

void USEREXECUTOR() // USEREXECUTOR
{
  Push(Read16(regsp)); // DUP
  CONSTANT(); // CONSTANT
  Push(Pop() + 2); //  2+
  CODE(); // (;CODE) inlined assembler code
// 0x17b7: inc    bx
// 0x17b8: inc    bx
// 0x17b9: mov    bx,[bx]
// 0x17bb: mov    bx,[bx+di]
// 0x17bd: dec    bx
// 0x17be: dec    bx
// 0x17bf: jmp    word ptr [bx]
}


// ================================================
// 0x17c1: WORD '+FIND' codep=0x224c wordp=0x17cb
// ================================================

void _plus_FIND() // +FIND
{
  HERE(); // HERE
  Push(Read16(user_CURRENT)); // CURRENT @
  _bo__dash_FIND_bc_(); // [-FIND]
}


// ================================================
// 0x17d5: WORD '(-FIND)' codep=0x224c wordp=0x17e1
// ================================================

void _ro__dash_FIND_rc_() // (-FIND)
{
  HERE(); // HERE
  Push(Read16(user_CONTEXT_1)); // CONTEXT_1 @
  _bo__dash_FINDS_bc_(); // [-FINDS]
  Push(Read16(regsp)); // DUP
  Push(Pop()==0?1:0); //  0=
  if (Pop() == 0) return;
  Push(!(Read16(user_CONTEXT_1)==Read16(user_CURRENT)?1:0)); // CONTEXT_1 @ CURRENT @ = NOT
  if (Pop() == 0) return;
  Pop(); // DROP
  HERE(); // HERE
  Push(Read16(user_CURRENT)); // CURRENT @
  _bo__dash_FINDS_bc_(); // [-FINDS]
}


// ================================================
// 0x180d: WORD '(FIND)' codep=0x1818 wordp=0x1818
// ================================================
// 0x1818: pop    bx
// 0x1819: pop    cx
// 0x181a: push   si
// 0x181b: push   di
// 0x181c: mov    dx,7F3F
// 0x181f: jmp    1826
// 0x1822: dec    bx
// 0x1823: dec    bx
// 0x1824: mov    bx,[bx]
// 0x1826: or     bx,bx
// 0x1828: jz     1859
// 0x182a: mov    si,cx
// 0x182c: mov    di,bx
// 0x182e: lodsb
// 0x182f: mov    ah,[di]
// 0x1831: and    ah,dl
// 0x1833: cmp    ah,al
// 0x1835: jnz    1822
// 0x1837: inc    di
// 0x1838: lodsb
// 0x1839: mov    ah,[di]
// 0x183b: xor    ah,al
// 0x183d: jz     1837
// 0x183f: and    ah,dh
// 0x1841: jnz    1822
// 0x1843: mov    ax,di
// 0x1845: pop    di
// 0x1846: pop    si
// 0x1847: inc    ax
// 0x1848: inc    ax
// 0x1849: inc    ax
// 0x184a: push   ax
// 0x184b: sub    ax,ax
// 0x184d: inc    ax
// 0x184e: mov    dl,[bx]
// 0x1850: mov    dh,ah
// 0x1852: push   dx
// 0x1853: push   ax
// 0x1854: lodsw
// 0x1855: mov    bx,ax
// 0x1857: jmp    word ptr [bx]
// 0x1859: pop    di
// 0x185a: pop    si
// 0x185b: push   bx
// 0x185c: lodsw
// 0x185d: mov    bx,ax
// 0x185f: jmp    word ptr [bx]

// ================================================
// 0x1861: WORD '(FORGET)' codep=0x224c wordp=0x186e params=1 returns=0
// ================================================

void _ro_FORGET_rc_() // (FORGET)
{
  unsigned short int i, imax;
  Push(Read16(regsp)); // DUP
  Push(Read16(user_CONTEXT_1)); // CONTEXT_1 @
  U_st_(); // U<
  if (Pop() != 0)
  {
    _i_FORTH(); // 'FORTH
    Push(user_CONTEXT_1); // CONTEXT_1
    Store(); // !
  }
  Push(Read16(regsp)); // DUP
  Push(Read16(user_CURRENT)); // CURRENT @
  U_st_(); // U<
  if (Pop() != 0)
  {
    DEFINITIONS(); // DEFINITIONS
  }
  Push(Read16(user_VOC_dash_LINK)); // VOC-LINK @
  while(1)
  {
    _2DUP(); // 2DUP
    W1A6E(); // W1A6E
    if (Pop() == 0) break;

    Push(Read16(Pop())); //  @
  }
  Push(Read16(regsp)); // DUP
  Push(user_VOC_dash_LINK); // VOC-LINK
  Store(); // !
  while(1)
  {
    if (Read16(regsp) != 0) Push(Read16(regsp)); // ?DUP
    if (Pop() == 0) break;

    Push(0);
    OVER(); // OVER
    Push(Pop() + 2); //  2+
    Store(); // !
    _2DUP(); // 2DUP
    Push(Pop() + 4); //  4 +
    Push(8);
    OVER(); // OVER
    Push(Pop() + Pop()); // +
    SWAP(); // SWAP

    i = Pop();
    imax = Pop();
    do // (DO)
    {
      Push(Read16(i)); // I @
      while(1)
      {
        _2DUP(); // 2DUP
        Push(Pop() + 1); //  1+
        W1A6E(); // W1A6E
        if (Pop() == 0) break;

        Push(Read16(Pop() - 2)); //  2- @
      }
      Push(i); // I
      Store(); // !
      OVER(); // OVER
      Push(Pop() + 2); //  2+
      Push(Read16(regsp)); // DUP
      Push(Read16(Pop())); //  @
      Push(Read16(i)); // I @
      UMAX(); // UMAX
      SWAP(); // SWAP
      Store(); // !
      Push(2);
      i += Pop();
    } while(i<imax); // (/LOOP)

    Pop(); // DROP
    Push(Read16(Pop())); //  @
  }
  Push(Pop() - Read16(user_LFALEN)); //  LFALEN @ -
  Push(user_DP); // DP
  Store(); // !
}


// ================================================
// 0x190e: WORD '(LINKS)' codep=0x224c wordp=0x191a
// ================================================

void _ro_LINKS_rc_() // (LINKS)
{
  OVER(); // OVER
  Push(Read16(Pop())); //  @
  HERE(); // HERE
  SWAP(); // SWAP
  _co_(); // ,
  Push(-1);
  _co_(); // ,
  SWAP(); // SWAP
  _co_(); // ,
  SWAP(); // SWAP
  Store(); // !
}


// ================================================
// 0x1934: WORD '<NAME>' codep=0x224c wordp=0x193f
// ================================================

void _st_NAME_gt_() // <NAME>
{
  Push(Read16(cc_BL)); // BL
  Exec("WORD"); // call of word 0x1f06 '(WORD)'
  Push(Read16(user_LOWERCASE)); // LOWERCASE @
  if (Pop() != 0)
  {
    Pop(); // DROP
    return;
  }
  COUNT(); // COUNT
  _gt_UPPERCASE(); // >UPPERCASE
}


// ================================================
// 0x1957: WORD '?FIND' codep=0x224c wordp=0x1961
// ================================================

void IsFIND() // ?FIND
{
  _st_NAME_gt_(); // <NAME>
  Exec("_dash_FIND"); // call of word 0x17e1 '(-FIND)'
  Push(Pop()==0?1:0); //  0=
  ABORT("Not found", 9);// (ABORT")
  Pop(); // DROP
}


// ================================================
// 0x1977: WORD '?[PTR]' codep=0x224c wordp=0x1982 params=1 returns=1
// ================================================

void Is_bo_PTR_bc_() // ?[PTR]
{
  Push(Read16(Pop() + 2) + 1==0?1:0); //  2+ @ 1+ 0=
}


// ================================================
// 0x198c: WORD 'DEFINITIONS' codep=0x224c wordp=0x199c params=0 returns=0
// ================================================

void DEFINITIONS() // DEFINITIONS
{
  Push(Read16(user_CONTEXT_1)); // CONTEXT_1 @
  Push(user_CURRENT); // CURRENT
  Store(); // !
}


// ================================================
// 0x19a6: WORD 'EMPTY' codep=0x224c wordp=0x19b0 params=0 returns=0
// ================================================

void EMPTY() // EMPTY
{
  Push(user_LFALEN); // LFALEN
  Push(Read16(regsp)); // DUP
  Push(Read16(Pop())); //  @
  SWAP(); // SWAP
  Push(2);
  OVER(); // OVER
  Store(); // !
  Push(Read16(Read16(cc_INITIAL_dash_DP)) + 2); // INITIAL-DP @ 2+
  _ro_FORGET_rc_(); // (FORGET)
  Store(); // !
}


// ================================================
// 0x19ca: WORD 'FIND' codep=0x224c wordp=0x19d3
// ================================================

void FIND() // FIND
{
  _st_NAME_gt_(); // <NAME>
  Exec("_dash_FIND"); // call of word 0x17e1 '(-FIND)'
  if (Pop() != 0)
  {
    Pop(); // DROP
    CFA(); // CFA
    return;
  }
  Push(0);
}


// ================================================
// 0x19e7: WORD 'FORGET' codep=0x224c wordp=0x19f2
// ================================================

void FORGET() // FORGET
{
  _st_NAME_gt_(); // <NAME>
  Push(Read16(user_MSGFLAGS) & 0x0040); // MSGFLAGS @ 0x0040 AND
  if (Pop() != 0)
  {
    Exec("_dash_FIND"); // call of word 0x17e1 '(-FIND)'
    Push(Pop()==0?1:0); //  0=
    W0B5D(); // W0B5D
  } else
  {
    _plus_FIND(); // +FIND
    Push(Pop()==0?1:0); //  0=
    ABORT("Not in CURRENT vocabulary", 25);// (ABORT")
  }
  Pop(); // DROP
  NFA(); // NFA
  Push(Read16(regsp)); // DUP
  WITHIN_ask_(); // WITHIN?
  Push((Pop()==0?1:0)==0?1:0); //  0= 0=
  ABORT("Within protected dictionary", 27);// (ABORT")
  _ro_FORGET_rc_(); // (FORGET)
}


// ================================================
// 0x1a5a: WORD 'W1A5C' codep=0x224c wordp=0x1a5c
// ================================================

void W1A5C() // W1A5C
{
  Push(Read16(user_CURRENT)); // CURRENT @
  _ro_LINKS_rc_(); // (LINKS)
}


// ================================================
// 0x1a64: WORD 'W1A66' codep=0x224c wordp=0x1a66
// ================================================
// orphan

void W1A66() // W1A66
{
  _i_FORTH(); // 'FORTH
  _ro_LINKS_rc_(); // (LINKS)
}


// ================================================
// 0x1a6c: WORD 'W1A6E' codep=0x224c wordp=0x1a6e params=3 returns=2
// ================================================

void W1A6E() // W1A6E
{
  U_st_(); // U<
  OVER(); // OVER
  Push((Pop()==0?1:0) - 1); //  0= 1-
  Push(Pop() & Pop()); // AND
}


// ================================================
// 0x1a7a: WORD 'VOCABULARY' codep=0x224c wordp=0x1a89
// ================================================

void VOCABULARY() // VOCABULARY
{
  Exec("CREATE"); // call of word 0x1cbb '(CREATE)'
  HERE(); // HERE
  Push(0);
  _co_(); // ,
  HERE(); // HERE
  Push(Read16(user_VOC_dash_LINK)); // VOC-LINK @
  _co_(); // ,
  Push(user_VOC_dash_LINK); // VOC-LINK
  Store(); // !
  Push(0);
  _co_(); // ,
  Push(0);
  _co_(); // ,
  Push(0);
  _co_(); // ,
  Push(0);
  _co_(); // ,
  Push(0);
  _co_(); // ,
  Exec("LINKS_gt_"); // call of word 0x1a5c 'W1A5C'
  CODE(); // (;CODE) inlined assembler code
// 0x1ab5: call   1649
  Push(user_CONTEXT_1); // CONTEXT_1
  Store(); // !
}


// ================================================
// 0x1abe: WORD 'W1AC0' codep=0x1ac0 wordp=0x1ac0
// ================================================
// 0x1ac0: pop    bx
// 0x1ac1: xor    ax,ax
// 0x1ac3: mov    al,[bx]
// 0x1ac5: inc    bx
// 0x1ac6: add    al,[bx]
// 0x1ac8: and    ax,0006
// 0x1acb: add    ax,0006
// 0x1ace: push   ax
// 0x1acf: lodsw
// 0x1ad0: mov    bx,ax
// 0x1ad2: jmp    word ptr [bx]

// ================================================
// 0x1ad4: WORD 'WITHIN?' codep=0x224c wordp=0x1ae0 params=1 returns=1
// ================================================

void WITHIN_ask_() // WITHIN?
{
  Push(Read16(user_FENCE) + 1); // FENCE @ 1+
  Push(Read16(Read16(cc_INITIAL_dash_DP))); // INITIAL-DP @
  UMAX(); // UMAX
  ME(); // ME
  UMAX(); // UMAX
  OVER(); // OVER
  SWAP(); // SWAP
  U_st_(); // U<
  SP_at_(); // SP@
  ROT(); // ROT
  U_st_(); // U<
  Push(Pop() | Pop()); // OR
}


// ================================================
// 0x1b00: WORD '[-FIND]' codep=0x224c wordp=0x1b0c
// ================================================

void _bo__dash_FIND_bc_() // [-FIND]
{
  Push(Read16(regsp)); // DUP
  Is_bo_PTR_bc_(); // ?[PTR]
  if (Pop() != 0)
  {
    Pop(); Pop(); // 2DROP
    Push(0);
    return;
  }
  OVER(); // OVER
  W1AC0(); // W1AC0
  Push(Pop() + Pop()); // +
  Push(Read16(Pop())); //  @
  _ro_FIND_rc_(); // (FIND)
}


// ================================================
// 0x1b28: WORD '[-FINDS]' codep=0x224c wordp=0x1b35
// ================================================

void _bo__dash_FINDS_bc_() // [-FINDS]
{
  unsigned short int a, b, c;
  SWAP(); // SWAP
  a = Pop(); // >R
  b = Pop(); // >R
  while(1)
  {
    Push(a); // I'
    Push(b); // I
    Push(Read16(regsp)); // DUP
    Is_bo_PTR_bc_(); // ?[PTR]
    if (Pop() != 0)
    {
      Push(Read16(Pop() + 4)); //  4 + @
    }
    _bo__dash_FIND_bc_(); // [-FIND]
    Push(Read16(regsp)); // DUP
    Push((Pop()==0?1:0) & !(Read16(b)==0?1:0)); //  0= I @ 0= NOT AND
    if (Pop() == 0) break;

    Pop(); // DROP
    Push(Read16(b)); // R> @
    c = Pop(); // >R
  }
  Push(b); // R>
  Push(a); // R>
  Pop(); Pop(); // 2DROP
}


// ================================================
// 0x1b77: WORD ''_1' codep=0x224c wordp=0x1b7d
// ================================================

void _i__1() // '_1
{
  IsFIND(); // ?FIND
  LITERAL(); // LITERAL
}


// ================================================
// 0x1b83: WORD ''STREAM' codep=0x224c wordp=0x1b8f params=0 returns=1
// ================================================

void _i_STREAM() // 'STREAM
{
  Push(Read16(user_BLK)); // BLK @
  if (Read16(regsp) != 0) Push(Read16(regsp)); // ?DUP
  if (Pop() != 0)
  {
    BLOCK_1(); // BLOCK_1
  } else
  {
    Push(Read16(user_TIB)); // TIB @
  }
  Push(Pop() + Read16(user__gt_IN)); //  >IN @ +
}


// ================================================
// 0x1bab: WORD '(' codep=0x224c wordp=0x1bb1
// ================================================

void _ro_() // (
{
  Push(-1);
  Push(user__gt_IN); // >IN
  _plus__ex_(); // +!
  Push(0x0029);
  Exec("WORD"); // call of word 0x1f06 '(WORD)'
  Push((Read16(Pop())&0xFF) + 1); //  C@ 1+
  HERE(); // HERE
  Push(Pop() + Pop()); // +
  Push(!((Read16(Pop())&0xFF)==0x0029?1:0)); //  C@ 0x0029 = NOT
  W0B30(); // W0B30
}


// ================================================
// 0x1bd5: WORD '(.")' codep=0x224c wordp=0x1bde params=0 returns=0
// ================================================

void _ro__dot__qm__rc_() // (.")
{
  unsigned short int a;
  Push(Read16(callp0)); // R@
  COUNT(); // COUNT
  Push(Read16(regsp)); // DUP
  Push((Pop() + 1) + callp0); //  1+ R> +
  a = Pop(); // >R
  Exec("TYPE"); // call of word 0x2690 '(TYPE)'
}


// ================================================
// 0x1bf0: WORD '(;CODE)' codep=0x224c wordp=0x1bfc params=0 returns=0
// ================================================

void _ro__sc_CODE_rc_() // (;CODE)
{
  Push(callp0); // R>
  LATEST(); // LATEST
  PFA(); // PFA
  CFA(); // CFA
  Store(); // !
}


// ================================================
// 0x1c08: WORD '(ABORT")' codep=0x224c wordp=0x1c15 params=1 returns=0
// ================================================

void _ro_ABORT_qm__rc_() // (ABORT")
{
  unsigned short int a;
  if (Pop() != 0)
  {
    Exec("WHERE"); // call of word 0x1e68 '(WHERE)'
    Exec("CR"); // call of word 0x26ee '(CR)'
    Push(Read16(callp0)); // R@
    COUNT(); // COUNT
    Exec("TYPE"); // call of word 0x2690 '(TYPE)'
    SP_ex_(); // SP!
    QUIT(); // QUIT
    return;
  }
  Push(callp0); // R>
  Push(Read16(regsp)); // DUP
  Push(Read16(Pop())&0xFF); //  C@
  Push(Pop() + Pop()); // +
  Push(Pop() + 1); //  1+
  a = Pop(); // >R
}


// ================================================
// 0x1c39: WORD '(ABORT)' codep=0x224c wordp=0x1c45 params=0 returns=0
// ================================================

void _ro_ABORT_rc_() // (ABORT)
{
  SP_ex_(); // SP!
  Push(0);
  W0AD9(); // W0AD9
  QUIT(); // QUIT
}


// ================================================
// 0x1c4f: WORD '."' codep=0x224c wordp=0x1c56
// ================================================

void Draw_qm_() // ."
{
  _i_STREAM(); // 'STREAM
  Push((Read16(Pop())&0xFF)==0x0022?1:0); //  C@ 0x0022 =
  if (Pop() != 0)
  {
    Push(1);
    Push(user__gt_IN); // >IN
    _plus__ex_(); // +!
    return;
  }
  Push(0x0022);
  Push(Read16(user_STATE)); // STATE @
  if (Pop() != 0)
  {
    COMPILE("(.")"); // ' (.")
  }
  Exec("WORD"); // call of word 0x1f06 '(WORD)'
  Push(Read16(regsp)); // DUP
  Push((Read16(Pop())&0xFF) + 1); //  C@ 1+
  OVER(); // OVER
  Push(Pop() + Pop()); // +
  Push(!((Read16(Pop())&0xFF)==0x0022?1:0)); //  C@ 0x0022 = NOT
  W0B30(); // W0B30
  Push(Read16(user_STATE)); // STATE @
  if (Pop() != 0)
  {
    Push((Read16(Pop())&0xFF) + 1); //  C@ 1+
    ALLOT(); // ALLOT
    return;
  }
  COUNT(); // COUNT
  Exec("TYPE"); // call of word 0x2690 '(TYPE)'
}


// ================================================
// 0x1cae: WORD '(CREATE)' codep=0x224c wordp=0x1cbb
// ================================================

void _ro_CREATE_rc_() // (CREATE)
{
  unsigned short int a;
  HERE(); // HERE
  Push(0);
  _co_(); // ,
  _st_NAME_gt_(); // <NAME>
  W1FF4(); // W1FF4
  W20C6(); // W20C6
  W2057(); // W2057
  HERE(); // HERE
  Push(Read16(regsp)); // DUP
  Push((Read16(Pop())&0xFF) & 0x001f); //  C@ 0x001f AND
  _2DUP(); // 2DUP
  SWAP(); // SWAP
  C_ex__1(); // C!_1
  Push(Read16(user_WIDTH)); // WIDTH @
  MIN(); // MIN
  Push(1);
  MAX(); // MAX
  Push(Pop() + 1); //  1+
  ALLOT(); // ALLOT
  Push(Read16(regsp)); // DUP
  Push(0x0080);
  TOGGLE(); // TOGGLE
  HERE(); // HERE
  Push(Pop() - 1); //  1-
  Push(0x0080);
  TOGGLE(); // TOGGLE
  Push(2);
  ALLOT(); // ALLOT
  Push(Read16(regsp)); // DUP
  W1AC0(); // W1AC0
  Push(Read16(user_CURRENT)); // CURRENT @
  a = Pop(); // >R
  Push(Pop() + a); //  I +
  ROT(); // ROT
  OVER(); // OVER
  Push(Read16(Pop())); //  @
  SWAP(); // SWAP
  Store(); // !
  OVER(); // OVER
  SWAP(); // SWAP
  Store(); // !
  Push(a + 4); // R> 4 +
  Store(); // !
  CODE(); // (;CODE) inlined assembler code
// 0x1d29: inc    bx
// 0x1d2a: inc    bx
// 0x1d2b: push   bx
// 0x1d2c: lodsw
// 0x1d2d: mov    bx,ax
// 0x1d2f: jmp    word ptr [bx]
}


// ================================================
// 0x1d31: WORD '(EXPECT)' codep=0x224c wordp=0x1d3e params=2 returns=0
// ================================================

void _ro_EXPECT_rc_() // (EXPECT)
{
  unsigned short int i, imax, a;
  OVER(); // OVER
  Push(Pop() + Pop()); // +
  OVER(); // OVER

  i = Pop();
  imax = Pop();
  do // (DO)
  {
    Exec("KEY_1"); // call of word 0x25d7 '(KEY)'
    Push(Pop() & 0x00ff); //  0x00ff AND
    Push(Read16(regsp)); // DUP
    Push(Pop()==8?1:0); //  8 =
    OVER(); // OVER
    Push(Pop()==0x007f?1:0); //  0x007f =
    Push(Pop() | Pop()); // OR
    if (Pop() != 0)
    {
      Pop(); // DROP
      Push(Read16(regsp)); // DUP
      Push(Pop()==i?1:0); //  I =
      Push(Read16(regsp)); // DUP
      Push(Pop() + (i - 1)); //  R> 1- +
      a = Pop(); // >R
      if (Pop() != 0)
      {
        Exec("BELL"); // call of word 0x266b 'BEEP'
      } else
      {
        Exec("BS"); // call of word 0x26cf '(BS)'
        SPACE(); // SPACE
        Exec("BS"); // call of word 0x26cf '(BS)'
      }
      Push(0);
      Push(0x0020);
    } else
    {
      Push(Read16(regsp)); // DUP
      Push(Pop()==0x000d?1:0); //  0x000d =
      if (Pop() != 0)
      {
        Pop(); // DROP
        Push(0);
        Push(Read16(cc_BL)); // BL
        imax = i; // LEAVE
      } else
      {
        Push(Read16(regsp)); // DUP
      }
      if (Read16(regsp) != 0) Push(Read16(regsp)); // ?DUP
      if (Pop() != 0)
      {
        Exec("EMIT"); // call of word 0x2731 '(EMIT)'
        Push(1);
        SWAP(); // SWAP
      } else
      {
        Push(Read16(regsp)); // DUP
      }
    }
    Push(a); // I
    Store(); // !
    a += Pop();
  } while(a<); // (/LOOP)

  Pop(); // DROP
}


// ================================================
// 0x1dc8: WORD '(INTERPRET)' codep=0x224c wordp=0x1dd8
// ================================================

void _ro_INTERPRET_rc_() // (INTERPRET)
{
  while(1)
  {
    _st_NAME_gt_(); // <NAME>
    Exec("_dash_FIND"); // call of word 0x17e1 '(-FIND)'
    if (Pop() != 0)
    {
      Push(Read16(user_STATE)); // STATE @
      U_st_(); // U<
      if (Pop() != 0)
      {
        Push(Pop() - 2); //  2-
        _co_(); // ,
      } else
      {
        EXECUTE(); // EXECUTE
        W21C9(); // W21C9
      }
    } else
    {
      HERE(); // HERE
      Exec("NUMBER"); // call of word 0x13d1 '(NUMBER)'
      Push(Read16(user_DPL) + 1); // DPL @ 1+
      if (Pop() != 0)
      {
        _2LITERAL(); // 2LITERAL
      } else
      {
        Pop(); // DROP
        LITERAL(); // LITERAL
      }
    }
  }
}


// ================================================
// 0x1e18: WORD '(LOAD)' codep=0x224c wordp=0x1e23
// ================================================

void _ro_LOAD_rc_() // (LOAD)
{
  unsigned short int a, b;
  if (Read16(regsp) != 0) Push(Read16(regsp)); // ?DUP
  Push(Pop()==0?1:0); //  0=
  ABORT("Not loadable", 12);// (ABORT")
  Push(Read16(user_BLK)); // BLK @
  a = Pop(); // >R
  Push(Read16(user__gt_IN)); // >IN @
  b = Pop(); // >R
  Push(0);
  Push(user__gt_IN); // >IN
  Store(); // !
  Push(user_BLK); // BLK
  Store(); // !
  Exec("INTERPRET"); // call of word 0x1dd8 '(INTERPRET)'
  Push(b); // R>
  Push(user__gt_IN); // >IN
  Store(); // !
  Push(a); // R>
  Push(user_BLK); // BLK
  Store(); // !
}


// ================================================
// 0x1e5c: WORD '(WHERE)' codep=0x224c wordp=0x1e68 params=0 returns=0
// ================================================

void _ro_WHERE_rc_() // (WHERE)
{
  unsigned short int a, i, imax;
  Push(Read16(user_BLK)); // BLK @
  if (Read16(regsp) != 0) Push(Read16(regsp)); // ?DUP
  if (Pop() != 0)
  {
    Push(Read16(regsp)); // DUP
    Push(user_SCR); // SCR
    Store(); // !
    Exec("CR"); // call of word 0x26ee '(CR)'
    Exec("CR"); // call of word 0x26ee '(CR)'
    PRINT("Scr# ", 5); // (.")
    Push(Read16(regsp)); // DUP
    Draw(); // .
    Push(Read16(user__gt_IN)); // >IN @
    Push(0x03ff);
    MIN(); // MIN
    Push(Read16(regsp)); // DUP
    Push(user_R_n_); // R#
    Store(); // !
    Push(Read16(cc_C_slash_L)); // C/L
    _slash_MOD(); // /MOD
    Push(Read16(regsp)); // DUP
    PRINT("LINE# ", 6); // (.")
    Draw(); // .
    ROT(); // ROT
    Exec("CR"); // call of word 0x26ee '(CR)'
    Exec("CR"); // call of word 0x26ee '(CR)'
    DrawLINE(); // .LINE
    Push(Read16(user__gt_IN)); // >IN @
    Push(0x03ff);
    _gt_(); // >
    Push(Pop() + Pop()); // +
  } else
  {
    Push(Read16(user__gt_IN)); // >IN @
  }
  Exec("CR"); // call of word 0x26ee '(CR)'
  HERE(); // HERE
  Push(Read16(Pop())&0xFF); //  C@
  Push(Read16(regsp)); // DUP
  a = Pop(); // >R
  _dash_(); // -
  HERE(); // HERE
  Push(Read16(a)); // R@
  Push(Pop() + Pop()); // +
  Push((Read16(Pop() + 1)&0xFF)==0x0020?1:0); //  1+ C@ 0x0020 =
  if (Pop() != 0)
  {
    Push(Pop() - 1); //  1-
  }
  SPACES(); // SPACES

  i = 0;
  imax = a; // R>
  do // (DO)
  {
    Push(0x005e);
    Exec("EMIT"); // call of word 0x2731 '(EMIT)'
    i++;
  } while(i<imax); // (LOOP)

}


// ================================================
// 0x1efb: WORD '(WORD)' codep=0x224c wordp=0x1f06
// ================================================

void _ro_WORD_rc_() // (WORD)
{
  unsigned short int a;
  _i_STREAM(); // 'STREAM
  SWAP(); // SWAP
  ENCLOSE(); // ENCLOSE
  _2DUP(); // 2DUP
  _gt_(); // >
  if (Pop() != 0)
  {
    Pop(); Pop(); // 2DROP
    Pop(); Pop(); // 2DROP
    Push(0);
    HERE(); // HERE
    Store(); // !
  } else
  {
    Push(user__gt_IN); // >IN
    _plus__ex_(); // +!
    OVER(); // OVER
    _dash_(); // -
    Push(Read16(regsp)); // DUP
    a = Pop(); // >R
    HERE(); // HERE
    C_ex__1(); // C!_1
    Push(Pop() + Pop()); // +
    HERE(); // HERE
    Push(Pop() + 1); //  1+
    Push(a); // R>
    Push(Read16(regsp)); // DUP
    Push(0x00ff);
    _gt_(); // >
    Push(Pop() & (((Read16(user_MSGFLAGS) & 4)==0?1:0)==0?1:0)); //  MSGFLAGS @ 4 AND 0= 0= AND
    ABORT("Input > 255 characters", 22);// (ABORT")
    Push(Pop() + 1); //  1+
    CMOVE_1(); // CMOVE_1
  }
  HERE(); // HERE
}


// ================================================
// 0x1f73: WORD '+LOOP' codep=0x224c wordp=0x1f7d
// ================================================

void _plus_LOOP() // +LOOP
{
  Push(3);
  IsPAIRS(); // ?PAIRS
  COMPILE("(+LOOP)"); // ' (+LOOP)
  HERE(); // HERE
  _dash_(); // -
  _co_(); // ,
}


// ================================================
// 0x1f8f: WORD ',' codep=0x224c wordp=0x1f95
// ================================================

void _co_() // ,
{
  HERE(); // HERE
  Store(); // !
  Push(2);
  Push(user_DP); // DP
  _plus__ex_(); // +!
  W21C9(); // W21C9
}


// ================================================
// 0x1fa3: WORD '/LOOP' codep=0x224c wordp=0x1fad
// ================================================

void _slash_LOOP() // /LOOP
{
  Push(3);
  IsPAIRS(); // ?PAIRS
  COMPILE("(/LOOP)"); // ' (/LOOP)
  HERE(); // HERE
  _dash_(); // -
  _co_(); // ,
}


// ================================================
// 0x1fbf: WORD ';' codep=0x224c wordp=0x1fc5
// ================================================

void _sc_() // ;
{
  IsCSP(); // ?CSP
  COMPILE("EXIT"); // ' EXIT
  SMUDGE(); // SMUDGE
  _bo_(); // [
}


// ================================================
// 0x1fd1: WORD '?COMP' codep=0x224c wordp=0x1fdb params=0 returns=0
// ================================================

void IsCOMP() // ?COMP
{
  Push(!Read16(user_STATE)); // STATE @ NOT
  ABORT("Compile only", 12);// (ABORT")
}


// ================================================
// 0x1ff2: WORD 'W1FF4' codep=0x224c wordp=0x1ff4 params=0 returns=0
// ================================================

void W1FF4() // W1FF4
{
  HERE(); // HERE
  Push(-((Read16(Pop())&0xFF)==0?1:0) & (Read16(user_MSGFLAGS) & 0x0010)); //  C@ 0= NEGATE MSGFLAGS @ 0x0010 AND AND
  ABORT("You can't redefine 'return'", 27);// (ABORT")
}


// ================================================
// 0x2028: WORD '?CSP' codep=0x224c wordp=0x2031 params=0 returns=0
// ================================================

void IsCSP() // ?CSP
{
  SP_at_(); // SP@
  Push(Pop() - Read16(user_CSP)); //  CSP @ -
  ABORT("Definition not finished", 23);// (ABORT")
}


// ================================================
// 0x2055: WORD 'W2057' codep=0x224c wordp=0x2057 params=0 returns=0
// ================================================

void W2057() // W2057
{
  Push(Read16(user_CURRENT)); // CURRENT @
  Is_bo_PTR_bc_(); // ?[PTR]
  ABORT("No current vocabulary", 21);// (ABORT")
}


// ================================================
// 0x2077: WORD '?LOADING' codep=0x224c wordp=0x2084 params=0 returns=0
// ================================================

void IsLOADING() // ?LOADING
{
  Push(!Read16(user_BLK)); // BLK @ NOT
  ABORT("Loading only", 12);// (ABORT")
}


// ================================================
// 0x209b: WORD '?PAIRS' codep=0x224c wordp=0x20a6 params=2 returns=0
// ================================================

void IsPAIRS() // ?PAIRS
{
  _dash_(); // -
  ABORT("Conditionals not paired", 23);// (ABORT")
}


// ================================================
// 0x20c4: WORD 'W20C6' codep=0x224c wordp=0x20c6 params=0 returns=0
// ================================================

void W20C6() // W20C6
{
  Push(Read16(user_MSGFLAGS) & (1 | 2)); // MSGFLAGS @ 1 2 OR AND
  if (Pop() == 0) return;
  _plus_FIND(); // +FIND
  if (Pop() != 0)
  {
    Push(1);
  } else
  {
    Push(Read16(user_MSGFLAGS) & 2); // MSGFLAGS @ 2 AND
    if (Pop() != 0)
    {
      Exec("_dash_FIND"); // call of word 0x17e1 '(-FIND)'
    } else
    {
      Push(0);
    }
  }
  if (Pop() == 0) return;
  Pop(); Pop(); // 2DROP
  HERE(); // HERE
  COUNT(); // COUNT
  Exec("TYPE"); // call of word 0x2690 '(TYPE)'
  PRINT(" isn't unique ", 14); // (.")
}


// ================================================
// 0x211b: WORD 'ABORT"' codep=0x224c wordp=0x2126
// ================================================

void ABORT_qm_() // ABORT"
{
  IsCOMP(); // ?COMP
  COMPILE("(ABORT")"); // ' (ABORT")
  _i_STREAM(); // 'STREAM
  Push((Read16(Pop())&0xFF)==0x0022?1:0); //  C@ 0x0022 =
  if (Pop() != 0)
  {
    Push(1);
    Push(user__gt_IN); // >IN
    _plus__ex_(); // +!
    Push(0);
    C_co_(); // C,
    return;
  }
  Push(0x0022);
  Exec("WORD"); // call of word 0x1f06 '(WORD)'
  Push(Read16(regsp)); // DUP
  Push((Read16(Pop())&0xFF) + 1); //  C@ 1+
  SWAP(); // SWAP
  OVER(); // OVER
  Push(Pop() + Pop()); // +
  Push(!((Read16(Pop())&0xFF)==0x0022?1:0)); //  C@ 0x0022 = NOT
  W0B30(); // W0B30
  ALLOT(); // ALLOT
}


// ================================================
// 0x216a: WORD 'AGAIN' codep=0x224c wordp=0x2174
// ================================================

void AGAIN() // AGAIN
{
  Push(1);
  IsPAIRS(); // ?PAIRS
  COMPILE("BRANCH"); // ' BRANCH
  HERE(); // HERE
  _dash_(); // -
  _co_(); // ,
}


// ================================================
// 0x2184: WORD 'ALLOT' codep=0x224c wordp=0x218e params=1 returns=0
// ================================================

void ALLOT() // ALLOT
{
  Push(Read16(regsp)); // DUP
  W0AD9(); // W0AD9
  Push(user_DP); // DP
  _plus__ex_(); // +!
}


// ================================================
// 0x2198: WORD 'BEGIN' codep=0x224c wordp=0x21a2 params=0 returns=2
// ================================================

void BEGIN() // BEGIN
{
  IsCOMP(); // ?COMP
  HERE(); // HERE
  Push(1);
}


// ================================================
// 0x21aa: WORD 'C,' codep=0x224c wordp=0x21b1
// ================================================

void C_co_() // C,
{
  HERE(); // HERE
  C_ex__1(); // C!_1
  Push(1);
  ALLOT(); // ALLOT
}


// ================================================
// 0x21bb: WORD 'CFA' codep=0x224c wordp=0x21c3 params=1 returns=1
// ================================================

void CFA() // CFA
{
  Push(Pop() - 2); //  2-
}


// ================================================
// 0x21c7: WORD 'W21C9' codep=0x21c9 wordp=0x21c9
// ================================================
// 0x21c9: mov    ax,[0A0B] // #SPACE
// 0x21cc: add    ax,[di+04]
// 0x21cf: jo     21E0
// 0x21d1: cmp    ax,sp
// 0x21d3: ja     21E0
// 0x21d5: mov    ax,[di]
// 0x21d7: cmp    ax,sp
// 0x21d9: jb     21E0
// 0x21db: lodsw
// 0x21dc: mov    bx,ax
// 0x21de: jmp    word ptr [bx]
// 0x21e0: mov    bx,0B10
// 0x21e3: jmp    word ptr [bx]

// ================================================
// 0x21e5: WORD 'COMPILE' codep=0x224c wordp=0x21f1
// ================================================

void COMPILE() // COMPILE
{
  unsigned short int a;
  IsCOMP(); // ?COMP
  Push(callp0); // R>
  Push(Read16(regsp)); // DUP
  Push(Pop() + 2); //  2+
  a = Pop(); // >R
  Push(Read16(Pop())); //  @
  _co_(); // ,
}


// ================================================
// 0x2201: WORD 'CONSTANT' codep=0x224c wordp=0x220e
// ================================================

void CONSTANT() // CONSTANT
{
  Exec("CREATE"); // call of word 0x1cbb '(CREATE)'
  _co_(); // ,
  CODE(); // (;CODE) inlined assembler code
// 0x2214: inc    bx
// 0x2215: inc    bx
// 0x2216: push   word ptr [bx]
// 0x2218: lodsw
// 0x2219: mov    bx,ax
// 0x221b: jmp    word ptr [bx]
}


// ================================================
// 0x221d: WORD 'DO' codep=0x224c wordp=0x2224
// ================================================

void DO() // DO
{
  COMPILE("(DO)"); // ' (DO)
  HERE(); // HERE
  Push(3);
}


// ================================================
// 0x2230: WORD ':' codep=0x224c wordp=0x2236
// ================================================

void _c_() // :
{
  SP_at_(); // SP@
  Push(user_CSP); // CSP
  Store(); // !
  Push(Read16(user_CURRENT)); // CURRENT @
  Push(user_CONTEXT_1); // CONTEXT_1
  Store(); // !
  Exec("CREATE"); // call of word 0x1cbb '(CREATE)'
  SMUDGE(); // SMUDGE
  _bc_(); // ]
  CODE(); // (;CODE) inlined assembler code
// 0x224c: inc    bx
// 0x224d: inc    bx
// 0x224e: dec    bp
// 0x224f: dec    bp
// 0x2250: mov    [bp+00],si
// 0x2253: mov    si,bx
// 0x2255: lodsw
// 0x2256: mov    bx,ax
// 0x2258: jmp    word ptr [bx]
}


// ================================================
// 0x225a: WORD 'DOES>' codep=0x224c wordp=0x2264
// ================================================

void DOES_gt_() // DOES>
{
  IsCSP(); // ?CSP
  COMPILE("(;CODE)"); // ' (;CODE)
  Push(0x00e8);
  C_co_(); // C,
  Push(0x1649);
  HERE(); // HERE
  Push(Pop() + 2); //  2+
  _dash_(); // -
  _co_(); // ,
}


// ================================================
// 0x227e: WORD 'ELSE' codep=0x224c wordp=0x2287
// ================================================

void ELSE() // ELSE
{
  Push(2);
  IsPAIRS(); // ?PAIRS
  COMPILE("BRANCH"); // ' BRANCH
  HERE(); // HERE
  Push(0);
  _co_(); // ,
  SWAP(); // SWAP
  Push(2);
  THEN(); // THEN
  Push(2);
}


// ================================================
// 0x229f: WORD 'ENCLOSE' codep=0x22ab wordp=0x22ab params=2 returns=4
// ================================================
// 0x22ab: pop    ax
// 0x22ac: pop    bx
// 0x22ad: push   bx
// 0x22ae: sub    ah,ah
// 0x22b0: mov    dx,FFFF
// 0x22b3: dec    bx
// 0x22b4: inc    bx
// 0x22b5: inc    dx
// 0x22b6: cmp    al,[bx]
// 0x22b8: jz     22B4
// 0x22ba: push   dx
// 0x22bb: cmp    ah,[bx]
// 0x22bd: jnz    22C9
// 0x22bf: mov    ax,dx
// 0x22c1: inc    dx
// 0x22c2: push   dx
// 0x22c3: push   ax
// 0x22c4: lodsw
// 0x22c5: mov    bx,ax
// 0x22c7: jmp    word ptr [bx]
// 0x22c9: inc    bx
// 0x22ca: inc    dx
// 0x22cb: cmp    al,[bx]
// 0x22cd: jz     22DC
// 0x22cf: cmp    ah,[bx]
// 0x22d1: jnz    22C9
// 0x22d3: mov    ax,dx
// 0x22d5: push   dx
// 0x22d6: push   ax
// 0x22d7: lodsw
// 0x22d8: mov    bx,ax
// 0x22da: jmp    word ptr [bx]
// 0x22dc: mov    ax,dx
// 0x22de: inc    ax
// 0x22df: push   dx
// 0x22e0: push   ax
// 0x22e1: lodsw
// 0x22e2: mov    bx,ax
// 0x22e4: jmp    word ptr [bx]

// ================================================
// 0x22e6: WORD 'HERE' codep=0x224c wordp=0x22ef params=0 returns=1
// ================================================

void HERE() // HERE
{
  Push(Read16(user_DP)); // DP @
}


// ================================================
// 0x22f5: WORD 'HEX' codep=0x224c wordp=0x22fd params=0 returns=0
// ================================================

void HEX() // HEX
{
  Push(0x0010);
  Push(user_BASE); // BASE
  Store(); // !
}


// ================================================
// 0x2307: WORD 'IF' codep=0x224c wordp=0x230e
// ================================================

void IF() // IF
{
  COMPILE("0BRANCH"); // ' 0BRANCH
  HERE(); // HERE
  Push(0);
  _co_(); // ,
  Push(2);
}


// ================================================
// 0x231c: WORD 'IMMEDIATE' codep=0x224c wordp=0x232a params=0 returns=0
// ================================================

void IMMEDIATE() // IMMEDIATE
{
  LATEST(); // LATEST
  Push(0x0040);
  TOGGLE(); // TOGGLE
}


// ================================================
// 0x2334: WORD 'LATEST' codep=0x224c wordp=0x233f params=0 returns=1
// ================================================

void LATEST() // LATEST
{
  Push(Read16(Read16(user_CURRENT) + 4)); // CURRENT @ 4 + @
}


// ================================================
// 0x234d: WORD 'LFA' codep=0x224c wordp=0x2355 params=1 returns=1
// ================================================

void LFA() // LFA
{
  NFA(); // NFA
  Push(Pop() - 2); //  2-
}


// ================================================
// 0x235b: WORD 'LITERAL' codep=0x224c wordp=0x2367
// ================================================

void LITERAL() // LITERAL
{
  Push(Read16(user_STATE)); // STATE @
  if (Pop() != 0)
  {
    COMPILE("LIT"); // ' LIT
    _co_(); // ,
    return;
  }
  W21C9(); // W21C9
}


// ================================================
// 0x237d: WORD 'LOOP' codep=0x224c wordp=0x2386
// ================================================

void LOOP() // LOOP
{
  Push(3);
  IsPAIRS(); // ?PAIRS
  COMPILE("(LOOP)"); // ' (LOOP)
  HERE(); // HERE
  _dash_(); // -
  _co_(); // ,
}


// ================================================
// 0x2398: WORD '2LITERAL' codep=0x224c wordp=0x23a5
// ================================================

void _2LITERAL() // 2LITERAL
{
  Push(Read16(user_STATE)); // STATE @
  if (Pop() != 0)
  {
    COMPILE("2LIT"); // ' 2LIT
    SWAP(); // SWAP
    _co_(); // ,
    _co_(); // ,
    return;
  }
  W21C9(); // W21C9
}


// ================================================
// 0x23bf: WORD 'NFA' codep=0x224c wordp=0x23c7 params=1 returns=1
// ================================================

void NFA() // NFA
{
  Push(Pop() - 3); //  3 -
  Push(-1);
  TRAVERSE(); // TRAVERSE
}


// ================================================
// 0x23d5: WORD 'PAD' codep=0x224c wordp=0x23dd params=0 returns=1
// ================================================

void PAD() // PAD
{
  HERE(); // HERE
  Push(Pop() + 0x0044); //  0x0044 +
}


// ================================================
// 0x23e7: WORD 'PFA' codep=0x224c wordp=0x23ef params=1 returns=1
// ================================================

void PFA() // PFA
{
  Push(1);
  TRAVERSE(); // TRAVERSE
  Push(Pop() + 3); //  3 +
}


// ================================================
// 0x23fb: WORD 'QUERY' codep=0x224c wordp=0x2405 params=0 returns=0
// ================================================

void QUERY() // QUERY
{
  Push(Read16(user_TIB)); // TIB @
  Push(0x0050);
  Exec("EXPECT"); // call of word 0x1d3e '(EXPECT)'
  Push(0);
  Push(user__gt_IN); // >IN
  Store(); // !
}


// ================================================
// 0x2417: WORD 'QUIT' codep=0x224c wordp=0x2420 params=0 returns=0
// ================================================

void QUIT() // QUIT
{
  BYE_2(); // BYE_2
  Push(user_BLK); // BLK
  Store(); // !
  Push(Read16(user_STATE)); // STATE @
  _bo_(); // [
  if (Pop() != 0)
  {
    LATEST(); // LATEST
    Push((Read16(Pop())&0xFF) & 0x0020); //  C@ 0x0020 AND
    if (Pop() != 0)
    {
      LATEST(); // LATEST
      _ro_FORGET_rc_(); // (FORGET)
    }
  }
  while(1)
  {
    Exec("CR"); // call of word 0x26ee '(CR)'
    RP_ex_(); // RP!
    QUERY(); // QUERY
    Exec("INTERPRET"); // call of word 0x1dd8 '(INTERPRET)'
    Push(Read16(user_STATE)==0?1:0); // STATE @ 0=
    if (Pop() != 0)
    {
      W03C3(); // W03C3
    }
  }
}


// ================================================
// 0x245c: WORD 'REPEAT' codep=0x224c wordp=0x2467
// ================================================

void REPEAT() // REPEAT
{
  unsigned short int a, b;
  a = Pop(); // >R
  b = Pop(); // >R
  AGAIN(); // AGAIN
  Push(b); // R>
  Push(a - 2); // R> 2-
  THEN(); // THEN
}


// ================================================
// 0x2477: WORD 'SMUDGE' codep=0x224c wordp=0x2482 params=0 returns=0
// ================================================

void SMUDGE() // SMUDGE
{
  LATEST(); // LATEST
  Push(0x0020);
  TOGGLE(); // TOGGLE
}


// ================================================
// 0x248c: WORD 'TEXT' codep=0x224c wordp=0x2495
// ================================================

void TEXT() // TEXT
{
  HERE(); // HERE
  Push(Read16(cc_C_slash_L) + 1); // C/L 1+
  BLANK(); // BLANK
  Exec("WORD"); // call of word 0x1f06 '(WORD)'
  Push(Read16(cc_BL)); // BL
  OVER(); // OVER
  Push(Read16(regsp)); // DUP
  Push(Read16(Pop())&0xFF); //  C@
  Push(Pop() + Pop()); // +
  Push(Pop() + 1); //  1+
  C_ex__1(); // C!_1
  PAD(); // PAD
  Push(Read16(cc_C_slash_L) + 1); // C/L 1+
  CMOVE_1(); // CMOVE_1
}


// ================================================
// 0x24b7: WORD 'THEN' codep=0x224c wordp=0x24c0 params=2 returns=0
// ================================================

void THEN() // THEN
{
  IsCOMP(); // ?COMP
  Push(2);
  IsPAIRS(); // ?PAIRS
  HERE(); // HERE
  OVER(); // OVER
  _dash_(); // -
  SWAP(); // SWAP
  Store(); // !
}


// ================================================
// 0x24d2: WORD 'TRAVERSE' codep=0x224c wordp=0x24df params=2 returns=1
// ================================================

void TRAVERSE() // TRAVERSE
{
  SWAP(); // SWAP
  do
  {
    OVER(); // OVER
    Push(Pop() + Pop()); // +
    Push(0x007f);
    OVER(); // OVER
    Push(Read16(Pop())&0xFF); //  C@
    _st_(); // <
  } while(Pop() == 0);
  SWAP(); // SWAP
  Pop(); // DROP
}


// ================================================
// 0x24f9: WORD 'UNTIL' codep=0x224c wordp=0x2503
// ================================================

void UNTIL() // UNTIL
{
  Push(1);
  IsPAIRS(); // ?PAIRS
  COMPILE("0BRANCH"); // ' 0BRANCH
  HERE(); // HERE
  _dash_(); // -
  _co_(); // ,
}


// ================================================
// 0x2513: WORD 'VARIABLE' codep=0x224c wordp=0x2520
// ================================================

void VARIABLE() // VARIABLE
{
  Exec("CREATE"); // call of word 0x1cbb '(CREATE)'
  Push(2);
  ALLOT(); // ALLOT
}


// ================================================
// 0x2528: WORD 'WHILE' codep=0x224c wordp=0x2532
// ================================================

void WHILE() // WHILE
{
  IF(); // IF
  Push(Pop() + 2); //  2+
}


// ================================================
// 0x2538: WORD 'zero_length_unknown' codep=0x224c wordp=0x253e params=0 returns=0
// ================================================

void zero_length_unknown() // zero_length_unknown
{
  Push(Read16(user_BLK)); // BLK @
  if (Pop() != 0)
  {
    Push(Read16(user_STATE)); // STATE @
    W0B30(); // W0B30
  }
  Push(callp0); // R>
  Pop(); // DROP
}


// ================================================
// 0x2552: WORD '[' codep=0x224c wordp=0x2558 params=0 returns=0
// ================================================

void _bo_() // [
{
  Push(0);
  Push(user_STATE); // STATE
  Store(); // !
}


// ================================================
// 0x2560: WORD '[COMPILE]' codep=0x224c wordp=0x256e
// ================================================

void _bo_COMPILE_bc_() // [COMPILE]
{
  IsCOMP(); // ?COMP
  IsFIND(); // ?FIND
  CFA(); // CFA
  _co_(); // ,
}


// ================================================
// 0x2578: WORD '\' codep=0x224c wordp=0x257e params=0 returns=0
// ================================================

void _bs_() // \
{
  Push(Read16(user__gt_IN)); // >IN @
  Push(0x0040);
  _slash_MOD(); // /MOD
  SWAP(); // SWAP
  if (Pop() != 0)
  {
    Push(Pop() + 1); //  1+
  }
  Push(Pop() * 0x0040); //  0x0040 *
  Push(user__gt_IN); // >IN
  Store(); // !
}


// ================================================
// 0x259c: WORD ']' codep=0x224c wordp=0x25a2 params=0 returns=0
// ================================================

void _bc_() // ]
{
  Push(0x00c0);
  Push(user_STATE); // STATE
  Store(); // !
}


// ================================================
// 0x25ac: WORD '(?TERMINAL)' codep=0x25bc wordp=0x25bc
// ================================================
// 0x25bc: mov    ah,01
// 0x25be: int    16
// 0x25c0: mov    ax,0001
// 0x25c3: jnz    25C7
// 0x25c5: sub    ax,ax
// 0x25c7: push   ax
// 0x25c8: lodsw
// 0x25c9: mov    bx,ax
// 0x25cb: jmp    word ptr [bx]

// ================================================
// 0x25cd: WORD '(KEY)' codep=0x25d7 wordp=0x25d7 params=0 returns=1
// ================================================
// 0x25d7: mov    ah,01
// 0x25d9: int    16
// 0x25db: jz     25D7
// 0x25dd: sub    ax,ax
// 0x25df: int    16
// 0x25e1: mov    cx,ax
// 0x25e3: sub    ah,ah
// 0x25e5: or     al,al
// 0x25e7: jnz    25ED
// 0x25e9: mov    al,ch
// 0x25eb: mov    ah,01
// 0x25ed: push   ax
// 0x25ee: lodsw
// 0x25ef: mov    bx,ax
// 0x25f1: jmp    word ptr [bx]

// ================================================
// 0x25f3: WORD 'BEEPMS' codep=0x1d29 wordp=0x25fe
// ================================================
// 0x25fe: db 0xfa 0x00 '  '

// ================================================
// 0x2600: WORD 'BEEPTONE' codep=0x1d29 wordp=0x260d
// ================================================
// 0x260d: db 0xf4 0x01 '  '

// ================================================
// 0x260f: WORD 'TONE' codep=0x2618 wordp=0x2618 params=1 returns=0
// ================================================
// 0x2618: pop    cx
// 0x2619: mov    al,B6
// 0x261b: mov    dx,0043
// 0x261e: cli    
// 0x261f: out    dx,al
// 0x2620: dec    dx
// 0x2621: mov    al,cl
// 0x2623: out    dx,al
// 0x2624: mov    al,ch
// 0x2626: out    dx,al
// 0x2627: sti    
// 0x2628: lodsw
// 0x2629: mov    bx,ax
// 0x262b: jmp    word ptr [bx]

// ================================================
// 0x262d: WORD 'BEEPON_1' codep=0x2638 wordp=0x2638
// ================================================
// 0x2638: cli    
// 0x2639: mov    dx,0061
// 0x263c: in     al,dx
// 0x263d: or     al,03
// 0x2640: out    dx,al
// 0x2641: sti    
// 0x2642: lodsw
// 0x2643: mov    bx,ax
// 0x2645: jmp    word ptr [bx]

// ================================================
// 0x2647: WORD 'BEEPOFF' codep=0x2653 wordp=0x2653 params=0 returns=0
// ================================================
// 0x2653: cli    
// 0x2654: mov    dx,0061
// 0x2657: in     al,dx
// 0x2658: and    al,FC
// 0x265b: out    dx,al
// 0x265c: sti    
// 0x265d: lodsw
// 0x265e: mov    bx,ax
// 0x2660: jmp    word ptr [bx]

// ================================================
// 0x2662: WORD 'BEEP' codep=0x224c wordp=0x266b params=0 returns=0
// ================================================

void BEEP() // BEEP
{
  Push(Read16(pp_Get_co_0_star_1_sc_)); // @,0*1; @
  if (Pop() == 0) return;
  Push(Read16(pp_BEEPTONE)); // BEEPTONE @
  TONE(); // TONE
  BEEPON_1(); // BEEPON_1
  Push(Read16(pp_BEEPMS)); // BEEPMS @
  MS(); // MS
  BEEPOFF(); // BEEPOFF
}


// ================================================
// 0x2685: WORD '(TYPE)' codep=0x224c wordp=0x2690 params=2 returns=0
// ================================================

void _ro_TYPE_rc_() // (TYPE)
{
  unsigned short int a;
  while(1)
  {
    Push(Read16(regsp)); // DUP
    if (Pop() == 0) break;

    _2DUP(); // 2DUP
    Push(((Read16(user_COLMAX)&0xFF) - (Read16(user_COL)&0xFF)) + 1); // COLMAX C@ COL C@ - 1+
    UMIN(); // UMIN
    a = Pop(); // >R
    CUR_gt_ADDR(); // CUR>ADDR
    Push(Read16(a)); // R@
    _gt_VMOVE(); // >VMOVE
    Push(Read16(a)); // R@
    CURFWD(); // CURFWD
    Push(Read16(a)); // R@
    _dash_(); // -
    SWAP(); // SWAP
    Push(Pop() + a); //  R> +
    SWAP(); // SWAP
  }
  Pop(); Pop(); // 2DROP
}


// ================================================
// 0x26c6: WORD '(BS)' codep=0x224c wordp=0x26cf params=0 returns=0
// ================================================

void _ro_BS_rc_() // (BS)
{
  IsPOSITION(); // ?POSITION
  Push(Read16(regsp)); // DUP
  if (Pop() != 0)
  {
    Push(Pop() - 1); //  1-
    Exec("POSITION"); // call of word 0x2767 '(POSITION)'
    return;
  }
  Pop(); Pop(); // 2DROP
  BEEP(); // BEEP
}


// ================================================
// 0x26e5: WORD '(CR)' codep=0x224c wordp=0x26ee params=0 returns=0
// ================================================

void _ro_CR_rc_() // (CR)
{
  IsPOSITION(); // ?POSITION
  Pop(); // DROP
  Push(Read16(regsp)); // DUP
  Push(Pop()==(Read16(user_ROWMAX)&0xFF)?1:0); //  ROWMAX C@ =
  if (Pop() != 0)
  {
    Push(1);
    SCROLLUP(); // SCROLLUP
  } else
  {
    Push(Pop() + 1); //  1+
  }
  Push(0);
  Exec("POSITION"); // call of word 0x2767 '(POSITION)'
}

// 0x270e: mov    bx,[di+1A]
// 0x2711: mov    al,[di+1C]
// 0x2714: add    al,bl
// 0x2716: mov    bl,50
// 0x2718: mul    bl
// 0x271a: mov    bl,bh
// 0x271c: mov    bh,[di+1D]
// 0x271f: add    bl,bh
// 0x2721: sub    bh,bh
// 0x2723: add    ax,bx
// 0x2725: ret    


// ================================================
// 0x2726: WORD '(EMIT)' codep=0x224c wordp=0x2731 params=1 returns=0
// ================================================

void _ro_EMIT_rc_() // (EMIT)
{
  Push(Pop() + Read16(user_ATTRIBUTE)); //  ATTRIBUTE @ +
  CUR_gt_ADDR(); // CUR>ADDR
  V_ex_(); // V!
  Push(1);
  CURFWD(); // CURFWD
}


// ================================================
// 0x2741: WORD '(PAGE)' codep=0x224c wordp=0x274c params=0 returns=0
// ================================================

void _ro_PAGE_rc_() // (PAGE)
{
  Push(0);
  SCROLLUP(); // SCROLLUP
  Push(0);
  Push(0);
  _ro_POSITION_rc_(); // (POSITION)
}


// ================================================
// 0x2758: WORD '(POSITION)' codep=0x2767 wordp=0x2767 params=2 returns=0
// ================================================
// 0x2767: pop    ax
// 0x2768: mov    [di+1B],al
// 0x276b: mov    dl,al
// 0x276d: pop    ax
// 0x276e: mov    [di+1A],al
// 0x2771: mov    dh,al
// 0x2773: sub    bx,bx
// 0x2775: mov    ah,02
// 0x2777: push   bp
// 0x2778: push   si
// 0x2779: int    10
// 0x277b: pop    si
// 0x277c: pop    bp
// 0x277d: lodsw
// 0x277e: mov    bx,ax
// 0x2780: jmp    word ptr [bx]

// ================================================
// 0x2782: WORD '-TRAILING' codep=0x224c wordp=0x2790 params=2 returns=2
// ================================================

void _dash_TRAILING() // -TRAILING
{
  unsigned short int i, imax;
  Push(Read16(regsp)); // DUP

  i = 0;
  imax = Pop();
  do // (DO)
  {
    _2DUP(); // 2DUP
    Push(Pop() + Pop()); // +
    Push((Read16(Pop() - 1)&0xFF) - Read16(cc_BL)); //  1- C@ BL -
    if (Pop() != 0)
    {
      imax = i; // LEAVE
    } else
    {
      Push(Pop() - 1); //  1-
    }
    i++;
  } while(i<imax); // (LOOP)

}


// ================================================
// 0x27b4: WORD '.' codep=0x224c wordp=0x27ba
// ================================================

void Draw() // .
{
  S_dash__gt_D(); // S->D
  D_dot_(); // D.
}


// ================================================
// 0x27c0: WORD '.LINE' codep=0x224c wordp=0x27ca params=1 returns=0
// ================================================

void DrawLINE() // .LINE
{
  BLOCK_1(); // BLOCK_1
  SWAP(); // SWAP
  Push(Pop() * Read16(cc_C_slash_L)); //  C/L *
  Push(Pop() + Pop()); // +
  Push(Read16(cc_C_slash_L)); // C/L
  _dash_TRAILING(); // -TRAILING
  Exec("TYPE"); // call of word 0x2690 '(TYPE)'
}


// ================================================
// 0x27dc: WORD '.R' codep=0x224c wordp=0x27e3
// ================================================

void DrawR() // .R
{
  unsigned short int a;
  a = Pop(); // >R
  S_dash__gt_D(); // S->D
  Push(a); // R>
  D_dot_R(); // D.R
}


// ================================================
// 0x27ed: WORD '>VMOVE' codep=0x27f8 wordp=0x27f8
// ================================================
// 0x27f8: mov    ax,[di+18]
// 0x27fb: mov    es,ax
// 0x27fd: mov    ax,[di+20]
// 0x2800: mov    dx,di
// 0x2802: mov    bx,si
// 0x2804: pop    cx
// 0x2805: pop    di
// 0x2806: pop    si
// 0x2807: jcxz   2813
// 0x2809: mov    al,[si]
// 0x280b: es:    
// 0x280c: mov    [di],ax
// 0x280e: inc    si
// 0x280f: inc    di
// 0x2810: inc    di
// 0x2811: loop   2809
// 0x2813: mov    si,bx
// 0x2815: mov    di,dx
// 0x2817: lodsw
// 0x2818: mov    bx,ax
// 0x281a: jmp    word ptr [bx]

// ================================================
// 0x281c: WORD '?' codep=0x224c wordp=0x2822
// ================================================

void Is() // ?
{
  Push(Read16(Pop())); //  @
  Draw(); // .
}


// ================================================
// 0x2828: WORD '?POSITION' codep=0x2836 wordp=0x2836 params=0 returns=2
// ================================================
// 0x2836: sub    ax,ax
// 0x2838: mov    al,[di+1A]
// 0x283b: push   ax
// 0x283c: mov    al,[di+1B]
// 0x283f: push   ax
// 0x2840: lodsw
// 0x2841: mov    bx,ax
// 0x2843: jmp    word ptr [bx]

// ================================================
// 0x2845: WORD 'CUR>ADDR' codep=0x2852 wordp=0x2852 params=0 returns=1
// ================================================
// 0x2852: call   270E
// 0x2855: shl    ax,1
// 0x2857: push   ax
// 0x2858: lodsw
// 0x2859: mov    bx,ax
// 0x285b: jmp    word ptr [bx]

// ================================================
// 0x285d: WORD 'CURFWD' codep=0x224c wordp=0x2868 params=1 returns=0
// ================================================

void CURFWD() // CURFWD
{
  unsigned short int i, imax;

  i = 0;
  imax = Pop();
  do // (DO)
  {
    IsPOSITION(); // ?POSITION
    Push(Read16(regsp)); // DUP
    Push(Read16(user_COLMAX)); // COLMAX @
    _st_(); // <
    Push(!Pop()); //  NOT
    if (Pop() != 0)
    {
      Pop(); Pop(); // 2DROP
      Exec("CR"); // call of word 0x26ee '(CR)'
    } else
    {
      Push(Pop() + 1); //  1+
      Exec("POSITION"); // call of word 0x2767 '(POSITION)'
    }
    i++;
  } while(i<imax); // (LOOP)

}


// ================================================
// 0x288e: WORD 'D.' codep=0x224c wordp=0x2895
// ================================================

void D_dot_() // D.
{
  Push(0);
  D_dot_R(); // D.R
  SPACE(); // SPACE
}


// ================================================
// 0x289d: WORD 'D.R' codep=0x224c wordp=0x28a5
// ================================================

void D_dot_R() // D.R
{
  unsigned short int a;
  Push(3);
  IsENOUGH(); // ?ENOUGH
  a = Pop(); // >R
  SWAP(); // SWAP
  OVER(); // OVER
  Push(Read16(regsp)); // DUP
  D_plus__dash_(); // D+-
  _st__n_(); // <#
  _n_S(); // #S
  ROT(); // ROT
  SIGN(); // SIGN
  _n__gt_(); // #>
  Push(a); // R>
  OVER(); // OVER
  _dash_(); // -
  SPACES(); // SPACES
  Exec("TYPE"); // call of word 0x2690 '(TYPE)'
}


// ================================================
// 0x28cb: WORD 'ID.' codep=0x224c wordp=0x28d3
// ================================================

void ID_dot_() // ID.
{
  unsigned short int i, imax;
  HERE(); // HERE
  Push(0x0020);
  Push(0x005f);
  FILL_1(); // FILL_1
  Push(Read16(regsp)); // DUP
  Push(1);
  TRAVERSE(); // TRAVERSE
  OVER(); // OVER
  _dash_(); // -
  OVER(); // OVER
  Push(Pop() + 1); //  1+
  SWAP(); // SWAP

  i = 0;
  imax = Pop();
  do // (DO)
  {
    Push(i); // I
    OVER(); // OVER
    Push(Pop() + Pop()); // +
    Push((Read16(Pop())&0xFF) & 0x007f); //  C@ 0x007f AND
    Push(i); // I
    HERE(); // HERE
    Push(Pop() + Pop()); // +
    C_ex__1(); // C!_1
    i++;
  } while(i<imax); // (LOOP)

  Pop(); // DROP
  HERE(); // HERE
  SWAP(); // SWAP
  Push((Read16(Pop())&0xFF) & 0x001f); //  C@ 0x001f AND
  Exec("TYPE"); // call of word 0x2690 '(TYPE)'
}


// ================================================
// 0x291f: WORD 'LIST' codep=0x224c wordp=0x2928
// ================================================

void LIST() // LIST
{
  unsigned short int i, imax;
  Exec("CR"); // call of word 0x26ee '(CR)'
  Push(Read16(regsp)); // DUP
  Push(user_SCR); // SCR
  Store(); // !
  PRINT("Scr #", 5); // (.")
  Draw(); // .

  i = 0;
  imax = 0x0010;
  do // (DO)
  {
    Exec("CR"); // call of word 0x26ee '(CR)'
    Push(i); // I
    Push(3);
    DrawR(); // .R
    SPACE(); // SPACE
    Push(i); // I
    Push(Read16(user_SCR)); // SCR @
    DrawLINE(); // .LINE
    Exec("IsTERMINAL"); // call of word 0x25bc '(?TERMINAL)'
    if (Pop() != 0)
    {
      imax = i; // LEAVE
    }
    i++;
  } while(i<imax); // (LOOP)

  Exec("CR"); // call of word 0x26ee '(CR)'
}


// ================================================
// 0x2966: WORD 'SCROLLUP' codep=0x2973 wordp=0x2973 params=1 returns=0
// ================================================
// 0x2973: pop    ax
// 0x2974: mov    ah,06
// 0x2976: sub    cx,cx
// 0x2978: mov    dh,[di+1E]
// 0x297b: mov    dl,[di+1F]
// 0x297e: mov    bx,[di+20]
// 0x2981: push   bp
// 0x2982: push   si
// 0x2983: int    10
// 0x2985: pop    si
// 0x2986: pop    bp
// 0x2987: lodsw
// 0x2988: mov    bx,ax
// 0x298a: jmp    word ptr [bx]

// ================================================
// 0x298c: WORD 'SPACE' codep=0x224c wordp=0x2996 params=0 returns=0
// ================================================

void SPACE() // SPACE
{
  Push(Read16(cc_BL)); // BL
  Exec("EMIT"); // call of word 0x2731 '(EMIT)'
}


// ================================================
// 0x299c: WORD 'SPACES' codep=0x224c wordp=0x29a7 params=1 returns=0
// ================================================

void SPACES() // SPACES
{
  unsigned short int i, imax;
  Push(0);
  MAX(); // MAX
  if (Read16(regsp) != 0) Push(Read16(regsp)); // ?DUP
  if (Pop() == 0) return;

  i = 0;
  imax = Pop();
  do // (DO)
  {
    SPACE(); // SPACE
    i++;
  } while(i<imax); // (LOOP)

}


// ================================================
// 0x29bd: WORD 'U.' codep=0x224c wordp=0x29c4
// ================================================

void U_dot_() // U.
{
  Push(0);
  D_dot_(); // D.
}


// ================================================
// 0x29ca: WORD 'U.R' codep=0x224c wordp=0x29d2
// ================================================

void U_dot_R() // U.R
{
  Push(0);
  SWAP(); // SWAP
  D_dot_R(); // D.R
}


// ================================================
// 0x29da: WORD 'V@' codep=0x29e1 wordp=0x29e1 params=1 returns=0
// ================================================
// 0x29e1: mov    cx,ds
// 0x29e3: mov    ax,[di+18]
// 0x29e6: mov    ds,ax
// 0x29e8: pop    bx
// 0x29e9: mov    ax,[bx]
// 0x29eb: mov    ah,00
// 0x29ed: push   ax
// 0x29ee: mov    ds,cx
// 0x29f0: lodsw
// 0x29f1: mov    bx,ax
// 0x29f3: jmp    word ptr [bx]

// ================================================
// 0x29f5: WORD 'V!' codep=0x29fc wordp=0x29fc params=2 returns=0
// ================================================
// 0x29fc: mov    cx,ds
// 0x29fe: mov    ax,[di+18]
// 0x2a01: mov    ds,ax
// 0x2a03: pop    bx
// 0x2a04: pop    ax
// 0x2a05: mov    [bx],ax
// 0x2a07: mov    ds,cx
// 0x2a09: lodsw
// 0x2a0a: mov    bx,ax
// 0x2a0c: jmp    word ptr [bx]

// ================================================
// 0x2a0e: WORD 'VFILL' codep=0x2a18 wordp=0x2a18 params=3 returns=0
// ================================================
// 0x2a18: mov    ax,[di+18]
// 0x2a1b: mov    dx,di
// 0x2a1d: mov    es,ax
// 0x2a1f: pop    ax
// 0x2a20: pop    cx
// 0x2a21: pop    di
// 0x2a22: cld    
// 0x2a23: repz   
// 0x2a24: stosw
// 0x2a25: mov    di,dx
// 0x2a27: lodsw
// 0x2a28: mov    bx,ax
// 0x2a2a: jmp    word ptr [bx]

// ================================================
// 0x2a2c: WORD 'VMOVE' codep=0x2a36 wordp=0x2a36 params=3 returns=0
// ================================================
// 0x2a36: mov    ax,[di+18]
// 0x2a39: mov    dx,di
// 0x2a3b: mov    es,ax
// 0x2a3d: mov    ds,ax
// 0x2a3f: pop    cx
// 0x2a40: pop    di
// 0x2a41: mov    bx,si
// 0x2a43: pop    si
// 0x2a44: repz   
// 0x2a45: movsw
// 0x2a46: mov    si,bx
// 0x2a48: mov    di,dx
// 0x2a4a: mov    ax,cs
// 0x2a4c: mov    ds,ax
// 0x2a4e: cld    
// 0x2a4f: lodsw
// 0x2a50: mov    bx,ax
// 0x2a52: jmp    word ptr [bx]

// ================================================
// 0x2a54: WORD 'VMOVE-' codep=0x2a5f wordp=0x2a5f
// ================================================
// 0x2a5f: mov    bx,[2A34]
// 0x2a63: std    
// 0x2a64: jmp    word ptr [bx]

// ================================================
// 0x2a66: WORD 'MS/TICK' codep=0x2214 wordp=0x2a72
// ================================================
// 0x2a72: dw 0x01a0

// ================================================
// 0x2a74: WORD 'TICKS/ADJ' codep=0x2214 wordp=0x2a82
// ================================================
// 0x2a82: dw 0x01ae

// ================================================
// 0x2a84: WORD 'MS/ADJ' codep=0x2214 wordp=0x2a8f
// ================================================
// 0x2a8f: dw 0x01b5

// ================================================
// 0x2a91: WORD 'TIME' codep=0x2a9a wordp=0x2a9a params=0 returns=1
// ================================================
// 0x2a9a: mov    dx,0043
// 0x2a9d: sub    ax,ax
// 0x2a9f: cli    
// 0x2aa0: out    dx,al
// 0x2aa1: mov    dx,0040
// 0x2aa4: in     al,dx
// 0x2aa5: mov    cl,al
// 0x2aa7: in     al,dx
// 0x2aa8: mov    ch,al
// 0x2aaa: neg    cx
// 0x2aac: mov    al,ch
// 0x2aae: shr    ax,1
// 0x2ab0: or     ax,ax
// 0x2ab2: rcr    ax,1
// 0x2ab4: adc    al,ah
// 0x2ab6: mov    cx,[0193]
// 0x2aba: cmp    ax,cx
// 0x2abc: jge    2AC0
// 0x2abe: mov    cx,ax
// 0x2ac0: mov    ax,[018A] // (TIME)
// 0x2ac3: add    ax,cx
// 0x2ac5: mov    [018A],ax // (TIME)
// 0x2ac8: jae    2AD4
// 0x2aca: mov    ax,[0188] // (TIME)
// 0x2acd: adc    ax,0000
// 0x2ad1: mov    [0188],ax // (TIME)
// 0x2ad4: mov    ax,[0193]
// 0x2ad7: sub    ax,cx
// 0x2ad9: jge    2ADD
// 0x2adb: sub    ax,ax
// 0x2add: mov    [0193],ax
// 0x2ae0: sti    
// 0x2ae1: mov    ax,0188
// 0x2ae4: push   ax
// 0x2ae5: lodsw
// 0x2ae6: mov    bx,ax
// 0x2ae8: jmp    word ptr [bx]

// ================================================
// 0x2aea: WORD 'MS' codep=0x224c wordp=0x2af1 params=0 returns=1
// ================================================

void MS() // MS
{
  Push(0);
  TIME(); // TIME
  _2_at_(); // 2@
  D_plus_(); // D+
  DNEGATE(); // DNEGATE
  while(1)
  {
    _2DUP(); // 2DUP
    TIME(); // TIME
    _2_at_(); // 2@
    D_plus_(); // D+
    _0_st_(); // 0<
    if (Pop() == 0) break;

    Pop(); // DROP
  }
  Pop(); // DROP
  Pop(); Pop(); // 2DROP
}


// ================================================
// 0x2b15: WORD '#DRIVES' codep=0x2214 wordp=0x2b21
// ================================================
// 0x2b21: dw 0x0003

// ================================================
// 0x2b23: WORD '#FCBS' codep=0x2214 wordp=0x2b2d
// ================================================
// 0x2b2d: dw 0x0002

// ================================================
// 0x2b2f: WORD '#FILES' codep=0x2214 wordp=0x2b3a
// ================================================
// 0x2b3a: dw 0x0004

// ================================================
// 0x2b3c: WORD '#SEC/TRK' codep=0x2214 wordp=0x2b49
// ================================================
// 0x2b49: dw 0x0009

// ================================================
// 0x2b4b: WORD 'DOS-FILE' codep=0x2214 wordp=0x2b58
// ================================================
// 0x2b58: dw 0x0002

// ================================================
// 0x2b5a: WORD 'RETRIES' codep=0x2214 wordp=0x2b66
// ================================================
// 0x2b66: dw 0x0008

// ================================================
// 0x2b68: WORD 'SECORIGIN' codep=0x2214 wordp=0x2b76
// ================================================
// 0x2b76: dw 0x0001

// ================================================
// 0x2b78: WORD ''SVBUF' codep=0x1d29 wordp=0x2b83
// ================================================
// 0x2b83: db 0x50 0x7b 'P{'

// ================================================
// 0x2b85: WORD ''MTBUF' codep=0x1d29 wordp=0x2b90
// ================================================
// 0x2b90: db 0xd4 0xb1 '  '

// ================================================
// 0x2b92: WORD '#SECS' codep=0x1d29 wordp=0x2b9c
// ================================================
// 0x2b9c: db 0x02 0x00 '  '

// ================================================
// 0x2b9e: WORD ''FCB' codep=0x1d29 wordp=0x2ba7
// ================================================
// 0x2ba7: db 0xca 0x2d ' -'

// ================================================
// 0x2ba9: WORD ''CACHE' codep=0x1d29 wordp=0x2bb4
// ================================================
// 0x2bb4: db 0x98 0x00 '  '

// ================================================
// 0x2bb6: WORD ''DOSMOUNT' codep=0x1d29 wordp=0x2bc4
// ================================================
// 0x2bc4: db 0xda 0x46 ' F'

// ================================================
// 0x2bc6: WORD ''DOSUNMOUNT' codep=0x1d29 wordp=0x2bd6
// ================================================
// 0x2bd6: db 0x44 0x47 'DG'

// ================================================
// 0x2bd8: WORD 'REUSEC' codep=0x1d29 wordp=0x2be3
// ================================================
// 0x2be3: db 0x98 0x00 '  '

// ================================================
// 0x2be5: WORD '1STCACHEBUF' codep=0x1d29 wordp=0x2bf5
// ================================================
// 0x2bf5: db 0x8b 0x43 ' C'

// ================================================
// 0x2bf7: WORD 'BLKCACHE' codep=0x1d29 wordp=0x2c04
// ================================================
// 0x2c04: db 0x63 0x43 'cC'

// ================================================
// 0x2c06: WORD 'DEFAULTDRV' codep=0x1d29 wordp=0x2c15
// ================================================
// 0x2c15: db 0x2c 0x6f ',o'

// ================================================
// 0x2c17: WORD 'DISK-ERROR' codep=0x1d29 wordp=0x2c26
// ================================================
// 0x2c26: db 0x00 0x00 '  '

// ================================================
// 0x2c28: WORD 'DRIVE' codep=0x1d29 wordp=0x2c32
// ================================================
// 0x2c32: db 0x04 0x00 '  '

// ================================================
// 0x2c34: WORD 'DTA_1' codep=0x1d29 wordp=0x2c3c
// ================================================
// 0x2c3c: db 0x00 0x01 0x63 0x33 '  c3'

// ================================================
// 0x2c40: WORD 'FILE' codep=0x1d29 wordp=0x2c49
// ================================================
// 0x2c49: db 0x02 0x00 '  '

// ================================================
// 0x2c4b: WORD 'FRESH' codep=0x1d29 wordp=0x2c55
// ================================================
// 0x2c55: db 0x4c 0xc5 'L '

// ================================================
// 0x2c57: WORD 'HEAD' codep=0x1d29 wordp=0x2c60
// ================================================
// 0x2c60: db 0x00 0x00 '  '

// ================================================
// 0x2c62: WORD 'LPREV' codep=0x1d29 wordp=0x2c6c
// ================================================
// 0x2c6c: db 0xe0 0x42 ' B'

// ================================================
// 0x2c6e: WORD 'OFFSET' codep=0x1d29 wordp=0x2c79
// ================================================
// 0x2c79: db 0x68 0x01 'h '

// ================================================
// 0x2c7b: WORD 'PREV' codep=0x1d29 wordp=0x2c84
// ================================================
// 0x2c84: db 0xe0 0x42 ' B'

// ================================================
// 0x2c86: WORD 'SEC' codep=0x1d29 wordp=0x2c8e
// ================================================
// 0x2c8e: db 0x05 0x00 '  '

// ================================================
// 0x2c90: WORD 'SEGCACHE' codep=0x1d29 wordp=0x2c9d
// ================================================
// 0x2c9d: db 0x77 0x43 'wC'

// ================================================
// 0x2c9f: WORD 'SYSK' codep=0x1d29 wordp=0x2ca8
// ================================================
// 0x2ca8: db 0x80 0x02 '  '

// ================================================
// 0x2caa: WORD 'TRACK' codep=0x1d29 wordp=0x2cb4
// ================================================
// 0x2cb4: db 0x00 0x00 '  '

// ================================================
// 0x2cb6: WORD 'USE' codep=0x1d29 wordp=0x2cbe
// ================================================
// 0x2cbe: db 0x21 0x43 '!C'

// ================================================
// 0x2cc0: WORD '=DRIVESIZES' codep=0x1d29 wordp=0x2cd0
// ================================================
// 0x2cd0: db 0x68 0x01 0x68 0x01 0x00 0x50 'h h  P'

// ================================================
// 0x2cd6: WORD 'DRIVESIZES' codep=0x224c wordp=0x2ce5 params=1 returns=1
// ================================================

void DRIVESIZES() // DRIVESIZES
{
  Push(Read16(Pop() * 2 + pp__eq_DRIVESIZES)); //  2* =DRIVESIZES + @
}


// ================================================
// 0x2cef: WORD 'BARRAY' codep=0x224c wordp=0x2cfa params=2 returns=1
// ================================================

void BARRAY() // BARRAY
{
  Push(Pop() + Pop()); // +
}


// ================================================
// 0x2cfe: WORD 'WARRAY' codep=0x224c wordp=0x2d09 params=2 returns=1
// ================================================

void WARRAY() // WARRAY
{
  SWAP(); // SWAP
  Push(Pop() * 2); //  2*
  Push(Pop() + Pop()); // +
}


// ================================================
// 0x2d11: WORD '=DRIVENUMBERS' codep=0x1d29 wordp=0x2d23
// ================================================
// 0x2d23: db 0x00 0x00 0x00 0x00 '    '

// ================================================
// 0x2d27: WORD '=FCBPFAS' codep=0x1d29 wordp=0x2d34
// ================================================
// 0x2d34: db 0x00 0x27 0x00 0x00 0x9e 0x2d 0xca 0x2d ' '   - -'

// ================================================
// 0x2d3c: WORD '=FILEBLOCKS' codep=0x1d29 wordp=0x2d4c
// ================================================
// 0x2d4c: db 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 '        '

// ================================================
// 0x2d54: WORD '=FILETYPES' codep=0x1d29 wordp=0x2d63
// ================================================
// 0x2d63: db 0xff 0xff 0xff 0xff '    '

// ================================================
// 0x2d67: WORD '=OFFSETS' codep=0x1d29 wordp=0x2d74
// ================================================
// 0x2d74: db 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff '        '

// ================================================
// 0x2d7c: WORD '=R/W'S' codep=0x1d29 wordp=0x2d87
// ================================================
// 0x2d87: db 0x31 0x36 0x44 0x33 0xe3 0x43 0xe3 0x43 '16D3 C C'

// ================================================
// 0x2d8f: WORD 'FCB' codep=0x1d29 wordp=0x2d97
// ================================================
// 0x2d97: db 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x05 0x50 0x52 0x4f 0x54 0x45 0x43 0x54 0x20 0x42 0x4c 0x4b 0x00 0x00 0x00 0x04 0x00 0x6c 0x00 0x00 0x49 0x13 0x02 0x8e 0x03 0x04 0x42 0xef 0x25 0x0a 0x66 0x0b 0x1a 0x1a 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x05 0x53 0x54 0x41 0x52 0x46 0x4c 0x54 0x32 0x43 0x4f 0x4d 0x00 0x00 0xf0 0xd4 0x00 0x00 0x00 0x00 0xaa 0x14 0x85 0x8a 0x01 0x04 0x42 0x00 0x00 0x07 0x66 0x0e 0x00 0x00 0x00 0x00 0x00 '        PROTECT BLK     l  I     B % f              STARFLT2COM              B   f      '

// ================================================
// 0x2def: WORD 'FILENAMES' codep=0x1d29 wordp=0x2dfd
// ================================================
// 0x2dfd: db 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 '                                            '

// ================================================
// 0x2e29: WORD 'DRIVENUMBERS' codep=0x224c wordp=0x2e3a params=1 returns=1
// ================================================

void DRIVENUMBERS() // DRIVENUMBERS
{
  Push(pp__eq_DRIVENUMBERS); // =DRIVENUMBERS
  BARRAY(); // BARRAY
}


// ================================================
// 0x2e40: WORD 'FCBPFAS' codep=0x224c wordp=0x2e4c params=1 returns=1
// ================================================

void FCBPFAS() // FCBPFAS
{
  Push(pp__eq_FCBPFAS); // =FCBPFAS
  WARRAY(); // WARRAY
}


// ================================================
// 0x2e52: WORD 'FILEBLOCKS' codep=0x224c wordp=0x2e61 params=1 returns=1
// ================================================

void FILEBLOCKS() // FILEBLOCKS
{
  Push(pp__eq_FILEBLOCKS); // =FILEBLOCKS
  WARRAY(); // WARRAY
}


// ================================================
// 0x2e67: WORD 'FILETYPES' codep=0x224c wordp=0x2e75 params=1 returns=1
// ================================================

void FILETYPES() // FILETYPES
{
  Push(pp__eq_FILETYPES); // =FILETYPES
  BARRAY(); // BARRAY
}


// ================================================
// 0x2e7b: WORD 'OFFSETS' codep=0x224c wordp=0x2e87 params=1 returns=1
// ================================================

void OFFSETS() // OFFSETS
{
  Push(pp__eq_OFFSETS); // =OFFSETS
  WARRAY(); // WARRAY
}


// ================================================
// 0x2e8d: WORD 'R/W'S' codep=0x224c wordp=0x2e97 params=1 returns=1
// ================================================

void R_slash_W_i_S() // R/W'S
{
  Push(pp__eq_R_slash_W_i_S); // =R/W'S
  WARRAY(); // WARRAY
}


// ================================================
// 0x2e9d: WORD 'L@' codep=0x2ea4 wordp=0x2ea4 params=2 returns=1
// ================================================
// 0x2ea4: mov    dx,ds
// 0x2ea6: pop    bx
// 0x2ea7: pop    ds
// 0x2ea8: push   word ptr [bx]
// 0x2eaa: mov    ds,dx
// 0x2eac: lodsw
// 0x2ead: mov    bx,ax
// 0x2eaf: jmp    word ptr [bx]

// ================================================
// 0x2eb1: WORD 'L!' codep=0x2eb8 wordp=0x2eb8 params=3 returns=0
// ================================================
// 0x2eb8: mov    dx,ds
// 0x2eba: pop    bx
// 0x2ebb: pop    ds
// 0x2ebc: pop    word ptr [bx]
// 0x2ebe: mov    ds,dx
// 0x2ec0: lodsw
// 0x2ec1: mov    bx,ax
// 0x2ec3: jmp    word ptr [bx]

// ================================================
// 0x2ec5: WORD 'LC@' codep=0x2ecd wordp=0x2ecd params=2 returns=1
// ================================================
// 0x2ecd: mov    dx,ds
// 0x2ecf: pop    bx
// 0x2ed0: pop    ds
// 0x2ed1: xor    ax,ax
// 0x2ed3: mov    al,[bx]
// 0x2ed5: push   ax
// 0x2ed6: mov    ds,dx
// 0x2ed8: lodsw
// 0x2ed9: mov    bx,ax
// 0x2edb: jmp    word ptr [bx]

// ================================================
// 0x2edd: WORD 'LC!' codep=0x2ee5 wordp=0x2ee5 params=3 returns=0
// ================================================
// 0x2ee5: mov    dx,ds
// 0x2ee7: pop    bx
// 0x2ee8: pop    ds
// 0x2ee9: pop    ax
// 0x2eea: mov    [bx],al
// 0x2eec: mov    ds,dx
// 0x2eee: lodsw
// 0x2eef: mov    bx,ax
// 0x2ef1: jmp    word ptr [bx]

// ================================================
// 0x2ef3: WORD 'LCMOVE' codep=0x2efe wordp=0x2efe params=5 returns=0
// ================================================
// 0x2efe: mov    [bp-02],di
// 0x2f01: mov    ax,es
// 0x2f03: mov    dx,ds
// 0x2f05: mov    bx,si
// 0x2f07: pop    cx
// 0x2f08: pop    di
// 0x2f09: pop    es
// 0x2f0a: pop    si
// 0x2f0b: pop    ds
// 0x2f0c: push   dx
// 0x2f0d: shr    cx,1
// 0x2f0f: rcl    dx,1
// 0x2f11: or     cx,cx
// 0x2f13: jz     2F17
// 0x2f15: repz   
// 0x2f16: movsw
// 0x2f17: and    dx,01
// 0x2f1a: jz     2F1D
// 0x2f1c: movsb
// 0x2f1d: mov    si,bx
// 0x2f1f: pop    ds
// 0x2f20: mov    es,ax
// 0x2f22: mov    di,[bp-02]
// 0x2f25: lodsw
// 0x2f26: mov    bx,ax
// 0x2f28: jmp    word ptr [bx]

// ================================================
// 0x2f2a: WORD '{LXCHG}' codep=0x1d29 wordp=0x2f36 params=0 returns=1
// ================================================
// 0x2f36: push   cx
// 0x2f37: es:    
// 0x2f38: mov    cx,[bx]
// 0x2f3a: xchg   ax,bx
// 0x2f3c: es:    
// 0x2f3d: xchg   [bx],cx
// 0x2f3f: xchg   ax,bx
// 0x2f41: es:    
// 0x2f42: mov    [bx],cx
// 0x2f44: pop    cx
// 0x2f45: ret    


// ================================================
// 0x2f46: WORD 'LWSCAN' codep=0x2f51 wordp=0x2f51 params=4 returns=2
// ================================================
// 0x2f51: pop    ax
// 0x2f52: pop    cx
// 0x2f53: or     cx,cx
// 0x2f55: jnz    2F5D
// 0x2f57: add    sp,04
// 0x2f5a: push   cx
// 0x2f5b: jmp    2F75
// 0x2f5d: mov    dx,es
// 0x2f5f: mov    bx,di
// 0x2f61: pop    di
// 0x2f62: pop    es
// 0x2f63: repnz  
// 0x2f64: scasw
// 0x2f65: dec    di
// 0x2f66: dec    di
// 0x2f67: es:    
// 0x2f68: cmp    ax,[di]
// 0x2f6a: jnz    2F70
// 0x2f6c: push   di
// 0x2f6d: mov    cx,0001
// 0x2f70: push   cx
// 0x2f71: mov    di,bx
// 0x2f73: mov    es,dx
// 0x2f75: lodsw
// 0x2f76: mov    bx,ax
// 0x2f78: jmp    word ptr [bx]

// ================================================
// 0x2f7a: WORD '[SEGCACHE]' codep=0x224c wordp=0x2f89 params=0 returns=2
// ================================================

void _bo_SEGCACHE_bc_() // [SEGCACHE]
{
  Push(Read16(pp__i_CACHE) * 2); // 'CACHE @ 2*
  Push(Read16(pp_SEGCACHE)); // SEGCACHE @
  SWAP(); // SWAP
}


// ================================================
// 0x2f97: WORD '[BLKCACHE]' codep=0x224c wordp=0x2fa6 params=0 returns=2
// ================================================

void _bo_BLKCACHE_bc_() // [BLKCACHE]
{
  Push(Read16(pp__i_CACHE) * 2); // 'CACHE @ 2*
  Push(Read16(pp_BLKCACHE)); // BLKCACHE @
  SWAP(); // SWAP
}


// ================================================
// 0x2fb4: WORD 'UPDATE' codep=0x224c wordp=0x2fbf params=0 returns=0
// ================================================

void UPDATE() // UPDATE
{
  Push(0x00ff);
  Push(Read16(pp_LPREV)); // LPREV @
  Push(2);
  LC_ex_(); // LC!
}


// ================================================
// 0x2fcd: WORD 'REFRESH' codep=0x224c wordp=0x2fd9 params=0 returns=0
// ================================================

void REFRESH() // REFRESH
{
  Push(pp_FRESH); // FRESH
  ON_3(); // ON_3
}


// ================================================
// 0x2fdf: WORD 'MTBUF' codep=0x224c wordp=0x2fe9 params=1 returns=0
// ================================================

void MTBUF() // MTBUF
{
  unsigned short int a;
  a = Pop(); // >R
  Push(0);
  Push(a); // I
  Push(0);
  L_ex_(); // L!
  Push(0);
  Push(a); // I
  Push(2);
  LC_ex_(); // LC!
  Push(0x00ff);
  Push(a); // I
  Push(3);
  LC_ex_(); // LC!
  Push(-1);
  Push(a); // I
  Push(6);
  L_ex_(); // L!
  Push(0);
  Push(a); // R>
  Push(0x0408);
  L_ex_(); // L!
}


// ================================================
// 0x301f: WORD 'MTBUFFERS' codep=0x224c wordp=0x302d params=0 returns=0
// ================================================

void MTBUFFERS() // MTBUFFERS
{
  Push(Read16(pp_USE)); // USE @
  MTBUF(); // MTBUF
  Push(Read16(pp_PREV)); // PREV @
  MTBUF(); // MTBUF
}


// ================================================
// 0x303b: WORD '(BUFFER)' codep=0x3048 wordp=0x3048 params=1 returns=2
// ================================================
// 0x3048: pop    cx
// 0x3049: mov    dx,es
// 0x304b: xor    bx,bx
// 0x304d: xor    ax,ax
// 0x304f: push   word ptr [2C84] // PREV
// 0x3053: pop    es
// 0x3054: es:    
// 0x3055: cmp    [bx+06],cx
// 0x3058: jnz    3064
// 0x305a: es:    
// 0x305b: db     82
// 0x305c: jg     3061
// 0x305e: add    [di+01],dh
// 0x3061: inc    ax
// 0x3062: jmp    3084
// 0x3064: push   word ptr [2CBE] // USE
// 0x3068: pop    es
// 0x3069: es:    
// 0x306a: cmp    [bx+06],cx
// 0x306d: jnz    3084
// 0x306f: es:    
// 0x3070: db     82
// 0x3071: jg     3076
// 0x3073: add    [di+0E],dh
// 0x3076: inc    ax
// 0x3077: push   word ptr [2C84] // PREV
// 0x307b: push   es
// 0x307c: pop    word ptr [2C84] // PREV
// 0x3080: pop    word ptr [2CBE] // USE
// 0x3084: or     ax,ax
// 0x3086: jnz    308B
// 0x3088: push   cx
// 0x3089: jmp    3094
// 0x308b: push   es
// 0x308c: mov    cx,[2C84] // PREV
// 0x3090: mov    [2C6C],cx // LPREV
// 0x3094: push   ax
// 0x3095: mov    es,dx
// 0x3097: lodsw
// 0x3098: mov    bx,ax
// 0x309a: jmp    word ptr [bx]

// ================================================
// 0x309c: WORD 'ADVANCE' codep=0x30a8 wordp=0x30a8
// ================================================
// 0x30a8: pop    ax
// 0x30a9: or     ax,ax
// 0x30ab: jz     30C4
// 0x30ad: mov    bx,ax
// 0x30af: sub    bx,02
// 0x30b2: push   es
// 0x30b3: push   word ptr [2C04] // BLKCACHE
// 0x30b7: pop    es
// 0x30b8: call   2F36
// 0x30bb: push   word ptr [2C9D] // SEGCACHE
// 0x30bf: pop    es
// 0x30c0: call   2F36
// 0x30c3: pop    es
// 0x30c4: lodsw
// 0x30c5: mov    bx,ax
// 0x30c7: jmp    word ptr [bx]

// ================================================
// 0x30c9: WORD 'MTCACHE' codep=0x224c wordp=0x30d5 params=0 returns=0
// ================================================

void MTCACHE() // MTCACHE
{
  unsigned short int i, imax;
  Push(Read16(pp__n_CACHE)); // #CACHE @
  if (Read16(regsp) != 0) Push(Read16(regsp)); // ?DUP
  if (Pop() != 0)
  {

    i = 0;
    imax = Pop();
    do // (DO)
    {
      Push(i); // I
      Push(pp__i_CACHE); // 'CACHE
      Store(); // !
      _bo_SEGCACHE_bc_(); // [SEGCACHE]
      L_at_(); // L@
      MTBUF(); // MTBUF
      Push(-1);
      _bo_BLKCACHE_bc_(); // [BLKCACHE]
      L_ex_(); // L!
      i++;
    } while(i<imax); // (LOOP)

  }
  Push(Read16(pp__n_CACHE) - 1); // #CACHE @ 1-
  Push(pp_REUSEC); // REUSEC
  Store(); // !
}


// ================================================
// 0x3107: WORD 'INITCACHE' codep=0x224c wordp=0x3115 params=0 returns=0
// ================================================

void INITCACHE() // INITCACHE
{
  unsigned short int a, i, imax;
  Push(Read16(pp__n_CACHE)); // #CACHE @
  if (Pop() != 0)
  {
    Push(Read16(pp_BLKCACHE)); // BLKCACHE @
    Push(Read16(pp__n_CACHE) * 2); // #CACHE @ 2*
    Push(0x0010);
    _slash_(); // /
    Push(Pop() + 1); //  1+
    Push(Read16(regsp)); // DUP
    a = Pop(); // >R
    Push(Pop() + Pop()); // +
    Push(Read16(regsp)); // DUP
    Push(pp_SEGCACHE); // SEGCACHE
    Store(); // !
    Push(Pop() + a); //  R> +
    Push(pp__1STCACHEBUF); // 1STCACHEBUF
    Store(); // !

    i = 0;
    imax = Read16(pp__n_CACHE); // #CACHE @
    do // (DO)
    {
      Push(i * 0x0041 + Read16(pp__1STCACHEBUF)); // I 0x0041 * 1STCACHEBUF @ +
      Push(i); // I
      Push(pp__i_CACHE); // 'CACHE
      Store(); // !
      _bo_SEGCACHE_bc_(); // [SEGCACHE]
      L_ex_(); // L!
      i++;
    } while(i<imax); // (LOOP)

  }
  MTCACHE(); // MTCACHE
}


// ================================================
// 0x316b: WORD '(CACHE)' codep=0x224c wordp=0x3177 params=1 returns=3
// ================================================

void _ro_CACHE_rc_() // (CACHE)
{
  unsigned short int a;
  a = Pop(); // >R
  Push(Read16(pp_BLKCACHE)); // BLKCACHE @
  Push(0);
  Push(Read16(pp__n_CACHE)); // #CACHE @
  Push(a); // I
  LWSCAN(); // LWSCAN
  if (Pop() != 0)
  {
    Push(Read16(pp_SEGCACHE)); // SEGCACHE @
    OVER(); // OVER
    L_at_(); // L@
    SWAP(); // SWAP
    ADVANCE(); // ADVANCE
    Push(Read16(regsp)); // DUP
    Push(pp_LPREV); // LPREV
    Store(); // !
    Push(1);
  } else
  {
    Push(a); // I
    Push(0);
  }
  Push(a); // R>
  Pop(); // DROP
}


// ================================================
// 0x31ad: WORD 'CACHE>USE' codep=0x224c wordp=0x31bb params=1 returns=0
// ================================================

void CACHE_gt_USE() // CACHE>USE
{
  unsigned short int a;
  Push(Read16(pp_USE)); // USE @
  a = Pop(); // >R
  Push(0);
  Push(a); // I
  Push(0);
  Push(0x0410);
  LCMOVE(); // LCMOVE
  Push(0);
  Push(a); // R>
  Push(2);
  L_ex_(); // L!
}


// ================================================
// 0x31d7: WORD '?INBCACHE' codep=0x224c wordp=0x31e5 params=1 returns=1
// ================================================

void IsINBCACHE() // ?INBCACHE
{
  Push(Read16(regsp)); // DUP
  Push(0);
  L_at_(); // L@
  if (Read16(regsp) != 0) Push(Read16(regsp)); // ?DUP
  if (Pop() != 0)
  {
    Push(6);
    L_at_(); // L@
    SWAP(); // SWAP
    Push(6);
    L_at_(); // L@
    Push((Pop()==Pop())?1:0); // =
    return;
  }
  Pop(); // DROP
  Push(0);
}


// ================================================
// 0x320b: WORD '[BUF>CACHE]' codep=0x224c wordp=0x321b params=1 returns=0
// ================================================

void _bo_BUF_gt_CACHE_bc_() // [BUF>CACHE]
{
  Push(Read16(regsp)); // DUP
  Push(0);
  OVER(); // OVER
  Push(0);
  L_at_(); // L@
  Push(0);
  Push(0x0410);
  LCMOVE(); // LCMOVE
  Push(0);
  SWAP(); // SWAP
  Push(2);
  LC_ex_(); // LC!
}


// ================================================
// 0x3237: WORD 'BUF>CACHE' codep=0x224c wordp=0x3245 params=1 returns=0
// ================================================

void BUF_gt_CACHE() // BUF>CACHE
{
  Push(Read16(regsp)); // DUP
  IsINBCACHE(); // ?INBCACHE
  OVER(); // OVER
  Push(2);
  LC_at_(); // LC@
  Push(Pop() & Pop()); // AND
  if (Pop() != 0)
  {
    _bo_BUF_gt_CACHE_bc_(); // [BUF>CACHE]
    return;
  }
  Pop(); // DROP
}


// ================================================
// 0x325f: WORD 'SEC-I/O' codep=0x224c wordp=0x326b
// ================================================

void SEC_dash_I_slash_O() // SEC-I/O
{
  Push(Read16(pp__n_SECS)); // #SECS @
  Push(0x16b6);
  C_ex__1(); // C!_1
  Push(0x16b6 + 1); // 0x16b6 1+
  C_ex__1(); // C!_1
  Push(pp_DTA_1); // DTA_1
  _2_at_(); // 2@
  Push(0x16b8);
  Store(); // !
  Push(0x16c6);
  Store(); // !
  Push(Read16(pp_DRIVE)); // DRIVE @
  Push(0x16bc);
  C_ex__1(); // C!_1
  Push(Read16(pp_HEAD)); // HEAD @
  Push(0x16bc + 1); // 0x16bc 1+
  C_ex__1(); // C!_1
  Push(Read16(pp_TRACK)); // TRACK @
  Push(0x16ba + 1); // 0x16ba 1+
  C_ex__1(); // C!_1
  Push(Read16(pp_SEC)); // SEC @
  Push(0x16ba);
  C_ex__1(); // C!_1
  Push(0x0013);
  INTERRUPT(); // INTERRUPT
  Push(Read16(0x16b6 + 1)&0xFF); // 0x16b6 1+ C@
}


// ================================================
// 0x32c9: WORD 'T&SCALC' codep=0x224c wordp=0x32d5 params=1 returns=0
// ================================================

void T_and_SCALC() // T&SCALC
{
  Push(Pop() * 2); //  2*
  Push(Read16(cc__n_SEC_slash_TRK)); // #SEC/TRK
  _slash_MOD(); // /MOD
  Push(0x0028);
  _slash_MOD(); // /MOD
  Push(pp_HEAD); // HEAD
  Store(); // !
  Push(pp_TRACK); // TRACK
  Store(); // !
  Push(Pop() + Read16(cc_SECORIGIN)); //  SECORIGIN +
  Push(pp_SEC); // SEC
  Store(); // !
}


// ================================================
// 0x32f3: WORD '[R/W]' codep=0x224c wordp=0x32fd
// ================================================

void _bo_R_slash_W_bc_() // [R/W]
{
  unsigned short int a, b;
  Push(0);
  a = Pop(); // >R
  while(1)
  {
    Push(Read16(regsp)); // DUP
    SEC_dash_I_slash_O(); // SEC-I/O
    Push(Read16(regsp)); // DUP
    Push(Read16(cc_RETRIES)); // RETRIES
    Push(a); // R>
    Push(Read16(regsp)); // DUP
    Push(Pop() + 1); //  1+
    b = Pop(); // >R
    _gt_(); // >
    Push(-Pop()); //  NEGATE
    Push(Pop() & Pop()); // AND
    if (Pop() == 0) break;

    Pop(); // DROP
    Push(0);
    SEC_dash_I_slash_O(); // SEC-I/O
    Pop(); // DROP
  }
  Push(Pop() + b * 0x0100); //  R> 0x0100 * +
  Push(pp_DISK_dash_ERROR); // DISK-ERROR
  Store(); // !
}


// ================================================
// 0x3337: WORD '[DIRECT]' codep=0x224c wordp=0x3344
// ================================================

void _bo_DIRECT_bc_() // [DIRECT]
{
  SWAP(); // SWAP
  T_and_SCALC(); // T&SCALC
  Push(!Pop() + 2); //  NOT 2+
  Push(Read16(pp_SEC)==Read16(cc__n_SEC_slash_TRK)?1:0); // SEC @ #SEC/TRK =
  if (Pop() != 0)
  {
    Push(1);
  } else
  {
    Push(2);
  }
  Push(pp__n_SECS); // #SECS
  Store(); // !
  _bo_R_slash_W_bc_(); // [R/W]
  Push(Read16(pp__n_SECS)==1?1:0); // #SECS @ 1 =
  if (Pop() != 0)
  {
    Push(Read16(cc_SECORIGIN)); // SECORIGIN
    Push(pp_SEC); // SEC
    Store(); // !
    Push(1);
    Push(pp_TRACK); // TRACK
    _plus__ex_(); // +!
    Push(0x0200);
    Push(pp_DTA_1); // DTA_1
    _plus__ex_(); // +!
    _bo_R_slash_W_bc_(); // [R/W]
  }
  Pop(); // DROP
}


// ================================================
// 0x338c: WORD '<MOUNT>' codep=0x224c wordp=0x3398
// ================================================

void _st_MOUNT_gt_() // <MOUNT>
{
  unsigned short int a, b;
  a = Pop(); // >R
  b = Pop(); // >R
  Push(b); // I
  Push(a); // I'
  FILETYPES(); // FILETYPES
  C_ex__1(); // C!_1
  Push(a); // I'
  DRIVENUMBERS(); // DRIVENUMBERS
  C_ex__1(); // C!_1
  Push(pp_FILENAMES + a * 0x000b); // FILENAMES I' 0x000b * +
  Push(0x000b);
  CMOVE_1(); // CMOVE_1
  Push(a); // I'
  FCBPFAS(); // FCBPFAS
  Store(); // !
  Push(a); // I'
  OFFSETS(); // OFFSETS
  Store(); // !
  Push(a); // I'
  R_slash_W_i_S(); // R/W'S
  Store(); // !
  Push(b==1?1:0); // R> 1 =
  if (Pop() != 0)
  {
    Push(a); // I
    Push(Read16(pp__i_DOSMOUNT)); // 'DOSMOUNT @
    EXECUTE(); // EXECUTE
  } else
  {
    Push(0);
    Push(a); // I
    DRIVESIZES(); // DRIVESIZES
  }
  Push(a); // I
  FILEBLOCKS(); // FILEBLOCKS
  Store(); // !
  Push(a); // R>
  OFFSETS(); // OFFSETS
  Push(Read16(Pop())); //  @
  Push(pp_OFFSET); // OFFSET
  Store(); // !
}


// ================================================
// 0x33fc: WORD 'DIRNAME' codep=0x1d29 wordp=0x3408
// ================================================
// 0x3408: db 0x2d 0x2d 0x64 0x69 0x72 0x65 0x63 0x74 0x2d 0x2d 0x20 '--direct-- '

// ================================================
// 0x3413: WORD 'DR0' codep=0x224c wordp=0x341b
// ================================================

void DR0() // DR0
{
  Push(0x3344); // '[DIRECT]'
  Push(0);
  Push(0);
  Push(pp_DIRNAME); // DIRNAME
  Push(1);
  Push(0);
  Push(0);
  _st_MOUNT_gt_(); // <MOUNT>
  Pop(); // DROP
}


// ================================================
// 0x3431: WORD 'DR1' codep=0x224c wordp=0x3439
// ================================================

void DR1() // DR1
{
  Push(0x3344); // '[DIRECT]'
  Push(0x0168);
  Push(0);
  Push(pp_DIRNAME); // DIRNAME
  Push(2);
  Push(0);
  Push(1);
  _st_MOUNT_gt_(); // <MOUNT>
  Pop(); // DROP
}


// ================================================
// 0x3451: WORD '<UNMOUNT>' codep=0x224c wordp=0x345f
// ================================================

void _st_UNMOUNT_gt_() // <UNMOUNT>
{
  unsigned short int a;
  FLUSH_1(); // FLUSH_1
  a = Pop(); // >R
  Push(a); // I
  FILETYPES(); // FILETYPES
  Push((Read16(Pop())&0xFF)==1?1:0); //  C@ 1 =
  if (Pop() != 0)
  {
    Push(a); // I
    Push(Read16(pp__i_DOSUNMOUNT)); // 'DOSUNMOUNT @
    EXECUTE(); // EXECUTE
  } else
  {
    Push(0);
  }
  Push(0);
  Push(a); // I
  DRIVENUMBERS(); // DRIVENUMBERS
  C_ex__1(); // C!_1
  Push(-1);
  Push(a); // I
  OFFSETS(); // OFFSETS
  Store(); // !
  Push(0);
  Push(a); // I
  FILEBLOCKS(); // FILEBLOCKS
  Store(); // !
  Push(0x00ff);
  Push(a); // I
  FILETYPES(); // FILETYPES
  C_ex__1(); // C!_1
  Push(pp_FILENAMES + a * 0x000b); // FILENAMES I 0x000b * +
  Push(0x000b);
  Push(Read16(cc_BL)); // BL
  FILL_1(); // FILL_1
  Push(a); // R>
  Pop(); // DROP
}


// ================================================
// 0x34bd: WORD 'UNMOUNT' codep=0x224c wordp=0x34c9
// ================================================

void UNMOUNT() // UNMOUNT
{
  _st_UNMOUNT_gt_(); // <UNMOUNT>
  ABORT("Unable to close.", 16);// (ABORT")
}


// ================================================
// 0x34e0: WORD 'NOFILES' codep=0x224c wordp=0x34ec
// ================================================

void NOFILES() // NOFILES
{
  unsigned short int i, imax;
  EMPTY_dash_BUFFERS(); // EMPTY-BUFFERS

  i = 0;
  imax = Read16(cc__n_FILES); // #FILES
  do // (DO)
  {
    Push(i); // I
    _st_UNMOUNT_gt_(); // <UNMOUNT>
    Pop(); // DROP
    i++;
  } while(i<imax); // (LOOP)

}


// ================================================
// 0x3500: WORD 'NODRIVES' codep=0x224c wordp=0x350d
// ================================================

void NODRIVES() // NODRIVES
{
  NOFILES(); // NOFILES
}


// ================================================
// 0x3511: WORD 'EMITS' codep=0x224c wordp=0x351b params=2 returns=0
// ================================================

void EMITS() // EMITS
{
  unsigned short int i, imax;
  if (Read16(regsp) != 0) Push(Read16(regsp)); // ?DUP
  if (Pop() != 0)
  {

    i = 0;
    imax = Pop();
    do // (DO)
    {
      Push(Read16(regsp)); // DUP
      Exec("EMIT"); // call of word 0x2731 '(EMIT)'
      i++;
    } while(i<imax); // (LOOP)

  }
  Pop(); // DROP
}


// ================================================
// 0x3531: WORD 'FILES' codep=0x224c wordp=0x353b
// ================================================

void FILES() // FILES
{
  unsigned short int i, imax;
  Exec("CR"); // call of word 0x26ee '(CR)'
  PRINT("File#    Name     Drv   #Blocks    At   ", 40); // (.")
  Exec("CR"); // call of word 0x26ee '(CR)'
  Push(0x002d);
  Push(0x002e);
  EMITS(); // EMITS

  i = 0;
  imax = Read16(cc__n_FILES); // #FILES
  do // (DO)
  {
    Exec("CR"); // call of word 0x26ee '(CR)'
    SPACE(); // SPACE
    Push(i); // I
    Push(2);
    DrawR(); // .R
    Push(3);
    SPACES(); // SPACES
    Push(i); // I
    FILETYPES(); // FILETYPES
    Push((Read16(Pop())&0xFF)==0x00ff?1:0); //  C@ 0x00ff =
    if (Pop() != 0)
    {
      PRINT("<unassigned>", 12); // (.")
    } else
    {
      Push(i * 0x000b + pp_FILENAMES); // I 0x000b * FILENAMES +
      Push(0x000b);
      Exec("TYPE"); // call of word 0x2690 '(TYPE)'
      SPACE(); // SPACE
      Push(i); // I
      DRIVENUMBERS(); // DRIVENUMBERS
      Push(((Read16(Pop())&0xFF) + 0x0041) - 1); //  C@ 0x0041 + 1-
      Exec("EMIT"); // call of word 0x2731 '(EMIT)'
      Push(0x003a);
      Exec("EMIT"); // call of word 0x2731 '(EMIT)'
      Push(4);
      SPACES(); // SPACES
      Push(i); // I
      FILEBLOCKS(); // FILEBLOCKS
      Push(Read16(Pop())); //  @
      Push(6);
      DrawR(); // .R
      Push(2);
      SPACES(); // SPACES
      Push(i); // I
      OFFSETS(); // OFFSETS
      Push(Read16(Pop())); //  @
      Push(Read16(regsp)); // DUP
      Push(6);
      DrawR(); // .R
      SPACE(); // SPACE
      Push(Pop()==Read16(pp_OFFSET)?1:0); //  OFFSET @ =
      if (Pop() != 0)
      {
        Push(0x001b);
        Exec("EMIT"); // call of word 0x2731 '(EMIT)'
        PRINT("OFFSET", 6); // (.")
      }
    }
    i++;
  } while(i<imax); // (LOOP)

  Exec("CR"); // call of word 0x26ee '(CR)'
  Push(0x002d);
  Push(0x002e);
  EMITS(); // EMITS
}


// ================================================
// 0x3628: WORD 'DRIVES' codep=0x224c wordp=0x3633
// ================================================

void DRIVES() // DRIVES
{
  FILES(); // FILES
}


// ================================================
// 0x3637: WORD '(DISKERROR?)' codep=0x224c wordp=0x3648 params=0 returns=0
// ================================================

void _ro_DISKERROR_ask__rc_() // (DISKERROR?)
{
  Push(Read16(pp_DISK_dash_ERROR)&0xFF); // DISK-ERROR C@
  IsUNRAVEL(); // ?UNRAVEL
}

// 0x3650: db 0x82 0x2c 0xae 0x0b 0xe7 0x2f 0x30 0x06 0x42 0x06 0x20 0x0f 0x24 0x2c 0x5e 0x0c 0x13 0x1c 0x0b 0x44 0x69 0x73 0x6b 0x20 0x65 0x72 0x72 0x6f 0x72 0x21 0x90 0x16 ' ,   /0 B   $,^    Disk error!  '

// ================================================
// 0x3670: WORD 'W3672' codep=0x3672 wordp=0x3672 params=1 returns=1
// ================================================
// 0x3672: pop    ax
// 0x3673: add    ax,[2C79] // OFFSET
// 0x3677: push   ax
// 0x3678: lodsw
// 0x3679: mov    bx,ax
// 0x367b: jmp    word ptr [bx]

// ================================================
// 0x367d: WORD 'W367F' codep=0x367f wordp=0x367f
// ================================================
// 0x367f: pop    dx
// 0x3680: push   di
// 0x3681: push   si
// 0x3682: push   bp
// 0x3683: mov    cx,[2B3A] // #FILES
// 0x3687: mov    di,2D74
// 0x368a: xor    bx,bx
// 0x368c: mov    bp,cx
// 0x368e: dec    bp
// 0x368f: shl    bp,1
// 0x3691: mov    si,di
// 0x3693: add    si,bp
// 0x3695: mov    ax,dx
// 0x3697: sub    ax,[si]
// 0x3699: js     36A8
// 0x369b: mov    si,2D4C
// 0x369e: add    si,bp
// 0x36a0: cmp    [si],ax
// 0x36a2: js     36A8
// 0x36a4: inc    bx
// 0x36a5: mov    cx,0001
// 0x36a8: loop   368C
// 0x36aa: shr    bp,1
// 0x36ac: mov    cx,bp
// 0x36ae: pop    bp
// 0x36af: pop    si
// 0x36b0: pop    di
// 0x36b1: push   ax
// 0x36b2: push   cx
// 0x36b3: push   bx
// 0x36b4: lodsw
// 0x36b5: mov    bx,ax
// 0x36b7: jmp    word ptr [bx]

// ================================================
// 0x36b9: WORD 'W36BB' codep=0x36bb wordp=0x36bb
// ================================================
// 0x36bb: xchg   ax,si
// 0x36bc: xor    cx,cx
// 0x36be: pop    si
// 0x36bf: add    si,2D23
// 0x36c3: mov    cl,[si]
// 0x36c5: or     cx,cx
// 0x36c7: jnz    36CD
// 0x36c9: mov    cx,[2C15] // DEFAULTDRV
// 0x36cd: dec    cx
// 0x36ce: push   cx
// 0x36cf: xchg   ax,si
// 0x36d0: lodsw
// 0x36d1: mov    bx,ax
// 0x36d3: jmp    word ptr [bx]

// ================================================
// 0x36d5: WORD 'W36D7' codep=0x224c wordp=0x36d7
// ================================================

void W36D7() // W36D7
{
  unsigned short int a;
  SWAP(); // SWAP
  W367F(); // W367F
  if (Pop() != 0)
  {
    Push(Read16(regsp)); // DUP
    Push(pp_FILE); // FILE
    Store(); // !
    Push(Read16(regsp)); // DUP
    W36BB(); // W36BB
    Push(pp_DRIVE); // DRIVE
    Store(); // !
    a = Pop(); // >R
    _2SWAP(); // 2SWAP
    Push(pp_DTA_1); // DTA_1
    _2_ex__1(); // 2!_1
    SWAP(); // SWAP
    Push(a); // R>
    R_slash_W_i_S(); // R/W'S
    Push(Read16(Pop())); //  @
    EXECUTE(); // EXECUTE
    return;
  }
  Pop(); Pop(); // 2DROP
  Pop(); Pop(); // 2DROP
  Pop(); // DROP
  Push(-1);
  Push(pp_DISK_dash_ERROR); // DISK-ERROR
  Store(); // !
}


// ================================================
// 0x3713: WORD 'W3715' codep=0x224c wordp=0x3715
// ================================================

void W3715() // W3715
{
  unsigned short int a, b;
  a = Pop(); // >R
  Push(Read16(pp_USE)); // USE @
  b = Pop(); // >R
  Push(b); // I
  Push(8);
  Push(a); // I'
  Push(1);
  Exec("R_slash_W"); // call of word 0x36d7 'W36D7'
  Push(b); // I
  Push(b); // I
  Push(6);
  L_ex_(); // L!
  Push(0);
  Push(b); // I
  Push(0);
  L_ex_(); // L!
  Push(0);
  Push(b); // I
  Push(2);
  L_ex_(); // L!
  Push(b); // R>
  Push(a); // R>
  SWAP(); // SWAP
  Push(6);
  L_ex_(); // L!
}


// ================================================
// 0x3751: WORD 'W3753' codep=0x224c wordp=0x3753 params=1 returns=0
// ================================================

void W3753() // W3753
{
  Push(Read16(regsp)); // DUP
  Push(2);
  LC_at_(); // LC@
  if (Pop() != 0)
  {
    Push(Read16(regsp)); // DUP
    Push(Read16(regsp)); // DUP
    Push(6);
    L_at_(); // L@
    Push(8);
    SWAP(); // SWAP
    Push(0);
    Exec("R_slash_W"); // call of word 0x36d7 'W36D7'
    Push(0);
    SWAP(); // SWAP
    Push(2);
    LC_ex_(); // LC!
    return;
  }
  Pop(); // DROP
}


// ================================================
// 0x3781: WORD 'W3783' codep=0x224c wordp=0x3783 params=0 returns=1
// ================================================

void W3783() // W3783
{
  Push(Read16(pp_REUSEC) + 1); // REUSEC @ 1+
  Push(Read16(regsp)); // DUP
  Push(Pop()==Read16(pp__n_CACHE)?1:0); //  #CACHE @ =
  if (Pop() != 0)
  {
    Pop(); // DROP
    Push(0);
  }
  Push(Read16(regsp)); // DUP
  Push(pp_REUSEC); // REUSEC
  Store(); // !
  Push(Read16(regsp)); // DUP
  Push(pp__i_CACHE); // 'CACHE
  Store(); // !
  _bo_SEGCACHE_bc_(); // [SEGCACHE]
  L_at_(); // L@
  W3753(); // W3753
}


// ================================================
// 0x37ad: WORD 'W37AF' codep=0x224c wordp=0x37af params=2 returns=0
// ================================================

void W37AF() // W37AF
{
  unsigned short int a;
  Push(pp__i_CACHE); // 'CACHE
  Store(); // !
  a = Pop(); // >R
  _bo_SEGCACHE_bc_(); // [SEGCACHE]
  L_at_(); // L@
  Push(a); // I
  Push(0);
  L_ex_(); // L!
  Push(a); // I
  Push(6);
  L_at_(); // L@
  _bo_BLKCACHE_bc_(); // [BLKCACHE]
  L_ex_(); // L!
  Push(a); // R>
  _bo_BUF_gt_CACHE_bc_(); // [BUF>CACHE]
}


// ================================================
// 0x37d1: WORD 'W37D3' codep=0x224c wordp=0x37d3 params=0 returns=0
// ================================================

void W37D3() // W37D3
{
  unsigned short int a;
  Push(Read16(pp_USE)); // USE @
  a = Pop(); // >R
  Push(a); // I
  Push(3);
  LC_at_(); // LC@
  Push(!Pop()); //  NOT
  if (Pop() != 0)
  {
    Push(Read16(pp__n_CACHE)); // #CACHE @
    if (Pop() != 0)
    {
      Push(a); // I
      IsINBCACHE(); // ?INBCACHE
      if (Pop() != 0)
      {
        Push(a); // I
        Push(2);
        LC_at_(); // LC@
        if (Pop() != 0)
        {
          Push(a); // I
          BUF_gt_CACHE(); // BUF>CACHE
        }
      } else
      {
        Push(a); // I
        W3783(); // W3783
        W37AF(); // W37AF
      }
    } else
    {
      Push(a); // I
      W3753(); // W3753
    }
    Push(a); // I
    MTBUF(); // MTBUF
  }
  Push(a); // R>
  Pop(); // DROP
}


// ================================================
// 0x3821: WORD 'W3823' codep=0x224c wordp=0x3823 params=1 returns=3
// ================================================

void W3823() // W3823
{
  _ro_BUFFER_rc_(); // (BUFFER)
  if (Pop() != 0)
  {
    Push(1);
    return;
  }
  Push(Read16(regsp)); // DUP
  W37D3(); // W37D3
  _ro_CACHE_rc_(); // (CACHE)
  if (Pop() != 0)
  {
    CACHE_gt_USE(); // CACHE>USE
    _ro_BUFFER_rc_(); // (BUFFER)
    return;
  }
  Pop(); // DROP
  Push(0);
}


// ================================================
// 0x3847: WORD 'W3849' codep=0x224c wordp=0x3849
// ================================================

void W3849() // W3849
{
  Push(Read16(regsp)); // DUP
  W3715(); // W3715
  _ro_BUFFER_rc_(); // (BUFFER)
  Pop(); // DROP
  Exec("DISKERROR_ask_"); // call of word 0x3648 '(DISKERROR?)'
}


// ================================================
// 0x3855: WORD 'BLOCK_1' codep=0x224c wordp=0x385f params=1 returns=2
// ================================================

void BLOCK_1() // BLOCK_1
{
  W3672(); // W3672
  W3823(); // W3823
  Push(!Pop()); //  NOT
  if (Pop() != 0)
  {
    W3849(); // W3849
  }
  SEG_gt_ADDR(); // SEG>ADDR
  Push(Pop() + 8); //  8 +
}


// ================================================
// 0x3875: WORD 'LBLOCK_1' codep=0x224c wordp=0x3880 params=1 returns=2
// ================================================

void LBLOCK_1() // LBLOCK_1
{
  W3672(); // W3672
  _ro_BUFFER_rc_(); // (BUFFER)
  Push(!Pop()); //  NOT
  if (Pop() != 0)
  {
    _ro_CACHE_rc_(); // (CACHE)
    Push(!Pop()); //  NOT
    if (Pop() != 0)
    {
      W37D3(); // W37D3
      W3849(); // W3849
    }
  }
  Push(8);
}


// ================================================
// 0x389c: WORD 'SAVE-BUFFERS' codep=0x224c wordp=0x38ad params=0 returns=0
// ================================================

void SAVE_dash_BUFFERS() // SAVE-BUFFERS
{
  unsigned short int i, imax;
  Push(Read16(pp__i_SVBUF)); // 'SVBUF @
  EXECUTE(); // EXECUTE
  Push(Read16(pp__n_CACHE)); // #CACHE @
  if (Read16(regsp) != 0) Push(Read16(regsp)); // ?DUP
  if (Pop() != 0)
  {
    Push(Read16(pp_USE)); // USE @
    BUF_gt_CACHE(); // BUF>CACHE
    Push(Read16(pp_PREV)); // PREV @
    BUF_gt_CACHE(); // BUF>CACHE

    i = 0;
    imax = Pop();
    do // (DO)
    {
      Push(i); // I
      Push(pp__i_CACHE); // 'CACHE
      Store(); // !
      _bo_SEGCACHE_bc_(); // [SEGCACHE]
      L_at_(); // L@
      W3753(); // W3753
      i++;
    } while(i<imax); // (LOOP)

  }
  Push(Read16(pp_USE)); // USE @
  W3753(); // W3753
  Push(Read16(pp_PREV)); // PREV @
  W3753(); // W3753
}


// ================================================
// 0x38eb: WORD 'EMPTY-BUFFERS' codep=0x224c wordp=0x38fd params=0 returns=0
// ================================================

void EMPTY_dash_BUFFERS() // EMPTY-BUFFERS
{
  Push(Read16(pp__i_MTBUF)); // 'MTBUF @
  EXECUTE(); // EXECUTE
  MTBUFFERS(); // MTBUFFERS
  MTCACHE(); // MTCACHE
}


// ================================================
// 0x3909: WORD 'FLUSH_1' codep=0x224c wordp=0x3913 params=0 returns=0
// ================================================

void FLUSH_1() // FLUSH_1
{
  SAVE_dash_BUFFERS(); // SAVE-BUFFERS
  EMPTY_dash_BUFFERS(); // EMPTY-BUFFERS
}


// ================================================
// 0x3919: WORD 'COPY' codep=0x224c wordp=0x3922 params=2 returns=2
// ================================================

void COPY() // COPY
{
  SWAP(); // SWAP
  BLOCK_1(); // BLOCK_1
  SWAP(); // SWAP
  BLOCK_1(); // BLOCK_1
  Push(0x0400);
  CMOVE_1(); // CMOVE_1
  UPDATE(); // UPDATE
}


// ================================================
// 0x3934: WORD 'BLOCKS' codep=0x224c wordp=0x393f params=3 returns=2
// ================================================

void BLOCKS() // BLOCKS
{
  unsigned short int a, i, imax, j, jmax;
  a = Pop(); // >R
  _2DUP(); // 2DUP
  _gt_(); // >
  if (Pop() != 0)
  {

    i = 0;
    imax = a; // R>
    do // (DO)
    {
      OVER(); // OVER
      Push(Pop() + i); //  I +
      OVER(); // OVER
      Push(Pop() + i); //  I +
      COPY(); // COPY
      i++;
    } while(i<imax); // (LOOP)

  } else
  {

    j = a - 1; // R> 1-
    jmax = 0;
    do // (DO)
    {
      OVER(); // OVER
      Push(Pop() + j); //  I +
      OVER(); // OVER
      Push(Pop() + j); //  I +
      COPY(); // COPY
      Push(-1);
      int step = Pop();
      j += step;
      if (((step>=0) && (j>=jmax)) || ((step<0) && (j<=jmax))) break;
    } while(1); // (+LOOP)

  }
  Pop(); Pop(); // 2DROP
  SAVE_dash_BUFFERS(); // SAVE-BUFFERS
}


// ================================================
// 0x3989: WORD 'PSW' codep=0x2214 wordp=0x3991
// ================================================
// 0x3991: dw 0x16b4

// ================================================
// 0x3993: WORD 'AX' codep=0x2214 wordp=0x399a
// ================================================
// 0x399a: dw 0x16b6

// ================================================
// 0x399c: WORD 'BX' codep=0x2214 wordp=0x39a3
// ================================================
// 0x39a3: dw 0x16b8

// ================================================
// 0x39a5: WORD 'CX' codep=0x2214 wordp=0x39ac
// ================================================
// 0x39ac: dw 0x16ba

// ================================================
// 0x39ae: WORD 'DX' codep=0x2214 wordp=0x39b5
// ================================================
// 0x39b5: dw 0x16bc

// ================================================
// 0x39b7: WORD 'BP' codep=0x2214 wordp=0x39be
// ================================================
// 0x39be: dw 0x16be

// ================================================
// 0x39c0: WORD 'DI' codep=0x2214 wordp=0x39c7
// ================================================
// 0x39c7: dw 0x16c0

// ================================================
// 0x39c9: WORD 'SI' codep=0x2214 wordp=0x39d0
// ================================================
// 0x39d0: dw 0x16c2

// ================================================
// 0x39d2: WORD 'DS' codep=0x2214 wordp=0x39d9
// ================================================
// 0x39d9: dw 0x16c4

// ================================================
// 0x39db: WORD 'ES' codep=0x2214 wordp=0x39e2
// ================================================
// 0x39e2: dw 0x16c6

// ================================================
// 0x39e4: WORD 'MATCH' codep=0x39ee wordp=0x39ee
// ================================================
// 0x39ee: pop    dx
// 0x39ef: pop    bx
// 0x39f0: pop    ax
// 0x39f1: pop    cx
// 0x39f2: push   di
// 0x39f3: push   si
// 0x39f4: push   cx
// 0x39f5: mov    di,cx
// 0x39f7: mov    cx,cs
// 0x39f9: mov    es,cx
// 0x39fb: cmp    ax,dx
// 0x39fd: jb     3A14
// 0x39ff: mov    cx,dx
// 0x3a01: mov    si,bx
// 0x3a03: repz   
// 0x3a04: cmpsb
// 0x3a05: jz     3A11
// 0x3a07: sub    ax,dx
// 0x3a09: add    ax,cx
// 0x3a0b: jnz    39FB
// 0x3a0d: mov    cx,cs
// 0x3a0f: jz     3A14
// 0x3a11: mov    ax,FFFF
// 0x3a14: mov    bx,di
// 0x3a16: add    ax,bx
// 0x3a18: pop    bx
// 0x3a19: sub    ax,bx
// 0x3a1b: pop    si
// 0x3a1c: pop    di
// 0x3a1d: sub    cx,0001
// 0x3a21: sbb    cx,cx
// 0x3a23: neg    cx
// 0x3a25: push   cx
// 0x3a26: push   ax
// 0x3a27: lodsw
// 0x3a28: mov    bx,ax
// 0x3a2a: jmp    word ptr [bx]

// ================================================
// 0x3a2c: WORD '@EXECUTE' codep=0x3a39 wordp=0x3a39
// ================================================
// 0x3a39: pop    bx
// 0x3a3a: mov    bx,[bx]
// 0x3a3c: dec    bx
// 0x3a3d: dec    bx
// 0x3a3e: jmp    word ptr [bx]

// ================================================
// 0x3a40: WORD 'NOP' codep=0x224c wordp=0x3a48 params=0 returns=0
// ================================================

void NOP() // NOP
{
}


// ================================================
// 0x3a4a: WORD '**' codep=0x224c wordp=0x3a51 params=0 returns=0
// ================================================

void _star__star_() // **
{
}


// ================================================
// 0x3a53: WORD ''UNRAVEL' codep=0x1d29 wordp=0x3a60
// ================================================
// 0x3a60: db 0x8a 0x8e '  '

// ================================================
// 0x3a62: WORD 'UNRAVEL' codep=0x224c wordp=0x3a6e params=0 returns=0
// ================================================

void UNRAVEL() // UNRAVEL
{
  MERR(); // MERR
  GetEXECUTE(); // @EXECUTE
  Push(callp0); // R>
  Pop(); // DROP
  Push(0x000f);
  do
  {
    Push(Read16(callp1 - 2)); // R> 2- @
    Exec("CR"); // call of word 0x26ee '(CR)'
    Push(Read16(regsp)); // DUP
    Push(0);
    Push(8);
    D_dot_R(); // D.R
    Push(3);
    SPACES(); // SPACES
    Push(Pop() + 2); //  2+
    NFA(); // NFA
    ID_dot_(); // ID.
    Push(Pop() - 1); //  1-
    if (Read16(regsp) != 0) Push(Read16(regsp)); // ?DUP
    Push(Pop()==0?1:0); //  0=
    RP_at_(); // RP@
    Push(Read16(user_R0)); // R0 @
    U_st_(); // U<
    Push(!Pop()); //  NOT
    Push(Pop() | Pop()); // OR
  } while(Pop() == 0);
  QUIT(); // QUIT
}


// ================================================
// 0x3ab2: WORD '?UNRAVEL' codep=0x224c wordp=0x3abf params=1 returns=0
// ================================================

void IsUNRAVEL() // ?UNRAVEL
{
  if (Pop() == 0) return;
  UNRAVEL(); // UNRAVEL
}


// ================================================
// 0x3ac7: WORD 'SYN' codep=0x224c wordp=0x3acf
// ================================================

void SYN() // SYN
{
  Exec("CREATE"); // call of word 0x1cbb '(CREATE)'
  FIND(); // FIND
  Push(Read16(regsp)); // DUP
  _co_(); // ,
  IMMEDIATE(); // IMMEDIATE
  Push(Pop() + 2); //  2+
  NFA(); // NFA
  Push((Read16(Pop())&0xFF) & 0x0040); //  C@ 0x0040 AND
  if (Pop() != 0)
  {
    CODE(); // (;CODE) inlined assembler code
// 0x3aeb: call   1649
    Push(Read16(Pop())); //  @
    CFAEXEC(); // CFAEXEC
    return;
  }
  CODE(); // (;CODE) inlined assembler code
// 0x3af8: call   1649
  Push(Read16(user_STATE)); // STATE @
  if (Pop() != 0)
  {
    Push(Read16(Pop())); //  @
    _co_(); // ,
    return;
  }
  Push(Read16(Pop())); //  @
  CFAEXEC(); // CFAEXEC
}


// ================================================
// 0x3b11: WORD 'TRACE' codep=0x1d29 wordp=0x3b1b
// ================================================
// 0x3b1b: db 0x00 0x00 '  '

// ================================================
// 0x3b1d: WORD '[TRACE' codep=0x1d29 wordp=0x3b28
// ================================================
// 0x3b28: db 0x48 0x3a 'H:'

// ================================================
// 0x3b2a: WORD 'TRACE]' codep=0x1d29 wordp=0x3b35
// ================================================
// 0x3b35: db 0x48 0x3a 'H:'

// ================================================
// 0x3b37: WORD 'D@' codep=0x3af8 wordp=0x3b3e
// ================================================
// 0x3b3e: db 0x38 0x0c '8 '

// ================================================
// 0x3b40: WORD 'V=' codep=0x3af8 wordp=0x3b47
// ================================================
// 0x3b47: db 0x1e 0x25 ' %'

// ================================================
// 0x3b49: WORD 'C=' codep=0x3af8 wordp=0x3b50
// ================================================
// 0x3b50: db 0x0c 0x22 ' "'

// ================================================
// 0x3b52: WORD '2V=' codep=0x224c wordp=0x3b5a
// ================================================

void _2V_eq_() // 2V=
{
  Exec("CREATE"); // call of word 0x1cbb '(CREATE)'
  Push(4);
  ALLOT(); // ALLOT
}


// ================================================
// 0x3b64: WORD '2C=' codep=0x224c wordp=0x3b6c
// ================================================

void _2C_eq_() // 2C=
{
  Exec("CREATE"); // call of word 0x1cbb '(CREATE)'
  _co_(); // ,
  _co_(); // ,
  CODE(); // (;CODE) inlined assembler code
// 0x3b74: call   1649
  _2_at_(); // 2@
}


// ================================================
// 0x3b7b: WORD '3' codep=0x2214 wordp=0x3b81
// ================================================
// 0x3b81: dw 0x0003

// ================================================
// 0x3b83: WORD '4' codep=0x2214 wordp=0x3b89
// ================================================
// 0x3b89: dw 0x0004

// ================================================
// 0x3b8b: WORD '5' codep=0x2214 wordp=0x3b91
// ================================================
// 0x3b91: dw 0x0005

// ================================================
// 0x3b93: WORD '6' codep=0x2214 wordp=0x3b99
// ================================================
// 0x3b99: dw 0x0006

// ================================================
// 0x3b9b: WORD '7' codep=0x2214 wordp=0x3ba1
// ================================================
// 0x3ba1: dw 0x0007

// ================================================
// 0x3ba3: WORD '8' codep=0x2214 wordp=0x3ba9
// ================================================
// 0x3ba9: dw 0x0008

// ================================================
// 0x3bab: WORD '9' codep=0x2214 wordp=0x3bb1
// ================================================
// 0x3bb1: dw 0x0009

// ================================================
// 0x3bb3: WORD '-1' codep=0x2214 wordp=0x3bba
// ================================================
// 0x3bba: dw 0xffff

// ================================================
// 0x3bbc: WORD '-2' codep=0x2214 wordp=0x3bc3
// ================================================
// 0x3bc3: dw 0xfffe

// ================================================
// 0x3bc5: WORD 'THRU_1' codep=0x224c wordp=0x3bce
// ================================================

void THRU_1() // THRU_1
{
  unsigned short int i, imax;
  _2DUP(); // 2DUP
  Push((Pop()==Pop())?1:0); // =
  if (Pop() != 0)
  {
    Pop(); // DROP
    Exec("LOAD_1"); // call of word 0x1e23 '(LOAD)'
    return;
  }
  Push(Pop() + 1); //  1+
  SWAP(); // SWAP

  i = Pop();
  imax = Pop();
  do // (DO)
  {
    Push(i); // I
    Exec("LOAD_1"); // call of word 0x1e23 '(LOAD)'
    i++;
  } while(i<imax); // (LOOP)

}


// ================================================
// 0x3bee: WORD '(TRACE)' codep=0x224c wordp=0x3bfa params=0 returns=0
// ================================================

void _ro_TRACE_rc_() // (TRACE)
{
}


// ================================================
// 0x3bfc: WORD 'OVA' codep=0x1d29 wordp=0x3c04
// ================================================
// 0x3c04: db 0x60 0xeb '` '

// ================================================
// 0x3c06: WORD 'T-DP' codep=0x1d29 wordp=0x3c0f
// ================================================
// 0x3c0f: db 0x3e 0xd5 0x27 0xf4 0x3e 0xd5 '> ' > '

// ================================================
// 0x3c15: WORD 'TRANS-ALLOT' codep=0x224c wordp=0x3c25 params=1 returns=0
// ================================================

void TRANS_dash_ALLOT() // TRANS-ALLOT
{
  Push(Read16(pp_OVA)); // OVA @
  SWAP(); // SWAP
  _dash_(); // -
  Push(pp_T_dash_DP); // T-DP
  Store(); // !
}


// ================================================
// 0x3c33: WORD 'NEWT-DP' codep=0x224c wordp=0x3c3f params=0 returns=0
// ================================================

void NEWT_dash_DP() // NEWT-DP
{
  Push(Read16(pp_T_dash_DP)); // T-DP @
  Push(pp_T_dash_DP + 4); // T-DP 4 +
  Store(); // !
}


// ================================================
// 0x3c4d: WORD 'TRANSIENT' codep=0x224c wordp=0x3c5b params=0 returns=0
// ================================================

void TRANSIENT() // TRANSIENT
{
  HERE(); // HERE
  Push(pp_T_dash_DP + 2); // T-DP 2+
  Store(); // !
  Push(Read16(pp_T_dash_DP + 4)); // T-DP 4 + @
  Push(user_DP); // DP
  Store(); // !
}


// ================================================
// 0x3c71: WORD 'RESIDENT' codep=0x224c wordp=0x3c7e params=0 returns=0
// ================================================

void RESIDENT() // RESIDENT
{
  HERE(); // HERE
  Push(pp_T_dash_DP + 4); // T-DP 4 +
  Store(); // !
  Push(Read16(pp_T_dash_DP + 2)); // T-DP 2+ @
  Push(user_DP); // DP
  Store(); // !
}


// ================================================
// 0x3c94: WORD 'HEAD:' codep=0x224c wordp=0x3c9e
// ================================================

void HEAD_c_() // HEAD:
{
  HERE(); // HERE
  TRANSIENT(); // TRANSIENT
  Exec("CREATE"); // call of word 0x1cbb '(CREATE)'
  _co_(); // ,
  IMMEDIATE(); // IMMEDIATE
  RESIDENT(); // RESIDENT
  CODE(); // (;CODE) inlined assembler code
// 0x3cac: call   1649
  Push(Read16(Pop())); //  @
  Push(Read16(user_STATE)); // STATE @
  if (Pop() != 0)
  {
    _co_(); // ,
    return;
  }
  Push(Pop() + 2); //  2+
  EXECUTE(); // EXECUTE
}


// ================================================
// 0x3cc5: WORD 'T:' codep=0x224c wordp=0x3ccc
// ================================================

void T_c_() // T:
{
  Push(0x224c);
  _co_(); // ,
  SP_at_(); // SP@
  Push(user_CSP); // CSP
  Store(); // !
  Push(Read16(user_CURRENT)); // CURRENT @
  Push(user_CONTEXT_1); // CONTEXT_1
  Store(); // !
  _bc_(); // ]
}


// ================================================
// 0x3ce4: WORD 'T;' codep=0x224c wordp=0x3ceb
// ================================================

void T_sc_() // T;
{
  IsCSP(); // ?CSP
  COMPILE("EXIT"); // ' EXIT
  _bo_(); // [
}


// ================================================
// 0x3cf5: WORD 'W3CF7' codep=0x224c wordp=0x3cf7 params=1 returns=0
// ================================================

void W3CF7() // W3CF7
{
  while(1)
  {
    Push(Read16(regsp)); // DUP
    Push(Read16(Pop() - 2)); //  2- @
    if (Read16(regsp) != 0) Push(Read16(regsp)); // ?DUP
    if (Pop() == 0) break;

    Push(Read16(regsp)); // DUP
    Push(Read16(pp_T_dash_DP)); // T-DP @
    U_st_(); // U<
    Push(!Pop()); //  NOT
    OVER(); // OVER
    Push(Read16(pp_OVA)); // OVA @
    U_st_(); // U<
    Push(Pop() & Pop()); // AND
    if (Pop() != 0)
    {
      Push(Read16(pp_TRACE)); // TRACE @
      if (Pop() != 0)
      {
        Push(Read16(regsp)); // DUP
        ID_dot_(); // ID.
        SPACE(); // SPACE
      }
      Push(Read16(Pop() - 2)); //  2- @
      OVER(); // OVER
      Push(Pop() - 2); //  2-
      Store(); // !
    } else
    {
      SWAP(); // SWAP
      Pop(); // DROP
    }
  }
  Pop(); // DROP
}


// ================================================
// 0x3d43: WORD 'W3D45' codep=0x224c wordp=0x3d45 params=0 returns=0
// ================================================

void W3D45() // W3D45
{
  Push(user_VOC_dash_LINK); // VOC-LINK
  while(1)
  {
    Push(Read16(regsp)); // DUP
    Push(Read16(Pop())); //  @
    if (Read16(regsp) != 0) Push(Read16(regsp)); // ?DUP
    if (Pop() == 0) break;

    Push(Read16(regsp)); // DUP
    Push(Read16(pp_T_dash_DP)); // T-DP @
    U_st_(); // U<
    Push(!Pop()); //  NOT
    OVER(); // OVER
    Push(Read16(pp_OVA)); // OVA @
    U_st_(); // U<
    Push(Pop() & Pop()); // AND
    if (Pop() != 0)
    {
      Push(Read16(pp_TRACE)); // TRACE @
      if (Pop() != 0)
      {
        Push(Read16(regsp)); // DUP
        Push(Pop() - 2); //  2-
        NFA(); // NFA
        ID_dot_(); // ID.
      }
      Push(Read16(Pop())); //  @
      OVER(); // OVER
      Store(); // !
    } else
    {
      SWAP(); // SWAP
      Pop(); // DROP
    }
  }
  Pop(); // DROP
}


// ================================================
// 0x3d8f: WORD 'DISPOSE' codep=0x224c wordp=0x3d9b
// ================================================

void DISPOSE() // DISPOSE
{
  unsigned short int i, imax;
  Exec("CR"); // call of word 0x26ee '(CR)'
  Push(Read16(user_VOC_dash_LINK) - 4); // VOC-LINK @ 4 -
  NFA(); // NFA
  ID_dot_(); // ID.
  Push(Read16(pp_T_dash_DP + 4) - Read16(pp_T_dash_DP)); // T-DP 4 + @ T-DP @ -
  PRINT(" Transbytes= ", 13); // (.")
  Draw(); // .
  NEWT_dash_DP(); // NEWT-DP
  W3D45(); // W3D45
  Push(user_VOC_dash_LINK); // VOC-LINK
  while(1)
  {
    Push(Read16(regsp)); // DUP
    Push(Read16(Pop())); //  @
    if (Read16(regsp) != 0) Push(Read16(regsp)); // ?DUP
    if (Pop() == 0) break;


    i = 0;
    imax = 4;
    do // (DO)
    {
      Push(i * 2 + 6); // I 2* 6 +
      OVER(); // OVER
      Push(Pop() + Pop()); // +
      W3CF7(); // W3CF7
      i++;
    } while(i<imax); // (LOOP)

    Pop(); // DROP
    Push(Read16(Pop())); //  @
  }
  Pop(); // DROP
}


// ================================================
// 0x3dfd: WORD ''_2' codep=0x224c wordp=0x3e03
// ================================================

void _i__2() // '_2
{
  IsFIND(); // ?FIND
  Push(Read16(regsp)); // DUP
  Push(Read16(Pop() - 2)); //  2- @
  Push(0x3c9e); // 'HEAD:'
  Push(Pop() + 0x000e); //  0x000e +
  Push((Pop()==Pop())?1:0); // =
  if (Pop() != 0)
  {
    Push(Read16(Pop()) + 2); //  @ 2+
  }
  LITERAL(); // LITERAL
}


// ================================================
// 0x3e23: WORD '(2V:)' codep=0x1d29 wordp=0x3e2d
// ================================================
// 0x3e2d: db 0x54 0x20 0x2d 0x20 'T - '

// ================================================
// 0x3e31: WORD '(C:)' codep=0x2214 wordp=0x3e3a
// ================================================
// 0x3e3a: dw 0x0000

// ================================================
// 0x3e3c: WORD '(2C:)' codep=0x3b74 wordp=0x3e46
// ================================================
// 0x3e46: dw 0x0000 0x0000

// ================================================
// 0x3e4a: WORD 'V:' codep=0x224c wordp=0x3e51
// ================================================

void V_c_() // V:
{
  HEAD_c_(); // HEAD:
  Push(0x1d29);
  _co_(); // ,
  Push(2);
  ALLOT(); // ALLOT
}


// ================================================
// 0x3e5f: WORD '2V:' codep=0x224c wordp=0x3e67
// ================================================

void _2V_c_() // 2V:
{
  HEAD_c_(); // HEAD:
  Push(0x1d29);
  _co_(); // ,
  Push(4);
  ALLOT(); // ALLOT
}


// ================================================
// 0x3e75: WORD 'C:' codep=0x224c wordp=0x3e7c
// ================================================

void C_c_() // C:
{
  HEAD_c_(); // HEAD:
  Push(0x2214);
  _co_(); // ,
  _co_(); // ,
}


// ================================================
// 0x3e88: WORD '2C:' codep=0x224c wordp=0x3e90
// ================================================

void _2C_c_() // 2C:
{
  HEAD_c_(); // HEAD:
  Push(0x3b74);
  _co_(); // ,
  _co_(); // ,
  _co_(); // ,
}


// ================================================
// 0x3e9e: WORD 'EXECUTES' codep=0x224c wordp=0x3eab params=2 returns=0
// ================================================

void EXECUTES() // EXECUTES
{
  Push(Read16(Pop())); //  @
  ME(); // ME
  Push(Pop() + Pop()); // +
  Store(); // !
}


// ================================================
// 0x3eb5: WORD 'BINARY' codep=0x224c wordp=0x3ec0 params=0 returns=0
// ================================================

void BINARY() // BINARY
{
  Push(2);
  Push(user_BASE); // BASE
  Store(); // !
}


// ================================================
// 0x3ec8: WORD 'H.' codep=0x224c wordp=0x3ecf
// ================================================

void H_dot_() // H.
{
  Push(Read16(user_BASE)); // BASE @
  SWAP(); // SWAP
  HEX(); // HEX
  U_dot_(); // U.
  Push(user_BASE); // BASE
  Store(); // !
}


// ================================================
// 0x3edf: WORD 'RANGE' codep=0x224c wordp=0x3ee9 params=2 returns=2
// ================================================

void RANGE() // RANGE
{
  OVER(); // OVER
  Push(Pop() + Pop()); // +
  SWAP(); // SWAP
}


// ================================================
// 0x3ef1: WORD 'ASCII' codep=0x224c wordp=0x3efb
// ================================================

void ASCII() // ASCII
{
  Push(Read16(cc_BL)); // BL
  Exec("WORD"); // call of word 0x1f06 '(WORD)'
  Push(Read16(Pop() + 1)&0xFF); //  1+ C@
  LITERAL(); // LITERAL
}


// ================================================
// 0x3f07: WORD 'SET_STR_AS_PARAM' codep=0x224c wordp=0x3f09 params=0 returns=2
// ================================================

void SET_STR_AS_PARAM() // SET_STR_AS_PARAM
{
  unsigned short int a;
  Push(Read16(callp0)); // R@
  COUNT(); // COUNT
  Push(Read16(regsp)); // DUP
  Push((Pop() + 1) + callp0); //  1+ R> +
  a = Pop(); // >R
}


// ================================================
// 0x3f19: WORD '"' codep=0x224c wordp=0x3f1f
// ================================================

void _qm_() // "
{
  Push(Read16(user_STATE)); // STATE @
  if (Pop() != 0)
  {
    COMPILE("SET_STR_AS_PARAM"); // ' SET_STR_AS_PARAM
  }
  Push(0x0022);
  Exec("WORD"); // call of word 0x1f06 '(WORD)'
  Push(Read16(user_STATE)); // STATE @
  if (Pop() != 0)
  {
    Push((Read16(Pop())&0xFF) + 1); //  C@ 1+
    ALLOT(); // ALLOT
    return;
  }
  Push(Read16(regsp)); // DUP
  Push((Read16(Pop())&0xFF) + 1); //  C@ 1+
  PAD(); // PAD
  SWAP(); // SWAP
  CMOVE_1(); // CMOVE_1
  PAD(); // PAD
  COUNT(); // COUNT
}


// ================================================
// 0x3f55: WORD 'DU<' codep=0x224c wordp=0x3f5d params=4 returns=1
// ================================================

void DU_st_() // DU<
{
  unsigned short int a, b;
  a = Pop(); // >R
  b = Pop(); // >R
  Push(Pop() + 0x8000); //  0x8000 +
  Push(b); // R>
  Push(a + 0x8000); // R> 0x8000 +
  D_st_(); // D<
}


// ================================================
// 0x3f75: WORD 'PICK' codep=0x224c wordp=0x3f7e params=1 returns=1
// ================================================

void PICK() // PICK
{
  Push(Read16(regsp)); // DUP
  Push(1);
  _st_(); // <
  if (Pop() != 0)
  {
    UNRAVEL(); // UNRAVEL
  }
  Push(Pop() * 2); //  2*
  SP_at_(); // SP@
  Push(Pop() + Pop()); // +
  Push(Read16(Pop())); //  @
}


// ================================================
// 0x3f94: WORD 'ROLL' codep=0x224c wordp=0x3f9d params=1 returns=0
// ================================================

void ROLL() // ROLL
{
  Push(Read16(regsp)); // DUP
  Push(1);
  _st_(); // <
  if (Pop() != 0)
  {
    UNRAVEL(); // UNRAVEL
  }
  Push(Pop() + 1); //  1+
  Push(Read16(regsp)); // DUP
  PICK(); // PICK
  SWAP(); // SWAP
  Push(Pop() * 2); //  2*
  SP_at_(); // SP@
  Push(Pop() + Pop()); // +
  do
  {
    Push(Read16(regsp)); // DUP
    Push(Read16(Pop() - 2)); //  2- @
    OVER(); // OVER
    Store(); // !
    Push(Pop() - 2); //  2-
    SP_at_(); // SP@
    OVER(); // OVER
    U_st_(); // U<
    Push(!Pop()); //  NOT
  } while(Pop() == 0);
  Pop(); Pop(); // 2DROP
}


// ================================================
// 0x3fd3: WORD '2OVER' codep=0x224c wordp=0x3fdd params=0 returns=2
// ================================================

void _2OVER() // 2OVER
{
  Push(4);
  PICK(); // PICK
  Push(4);
  PICK(); // PICK
}


// ================================================
// 0x3fe7: WORD 'D-' codep=0x224c wordp=0x3fee params=4 returns=2
// ================================================

void D_dash_() // D-
{
  DNEGATE(); // DNEGATE
  D_plus_(); // D+
}


// ================================================
// 0x3ff4: WORD 'D0=' codep=0x224c wordp=0x3ffc params=2 returns=1
// ================================================

void D0_eq_() // D0=
{
  Push(Pop() | Pop()); // OR
  Push(Pop()==0?1:0); //  0=
}


// ================================================
// 0x4002: WORD 'D=' codep=0x224c wordp=0x4009 params=4 returns=1
// ================================================

void D_eq_() // D=
{
  D_dash_(); // D-
  D0_eq_(); // D0=
}


// ================================================
// 0x400f: WORD 'D>' codep=0x224c wordp=0x4016 params=4 returns=1
// ================================================

void D_gt_() // D>
{
  _2SWAP(); // 2SWAP
  D_st_(); // D<
}


// ================================================
// 0x401c: WORD 'WITHIN' codep=0x224c wordp=0x4027 params=3 returns=1
// ================================================

void WITHIN() // WITHIN
{
  unsigned short int a;
  ROT(); // ROT
  a = Pop(); // >R
  Push(a); // I
  _gt_(); // >
  SWAP(); // SWAP
  Push(Pop() - 1); //  1-
  Push(a); // R>
  _st_(); // <
  Push(Pop() & Pop()); // AND
}


// ================================================
// 0x403b: WORD 'DMIN' codep=0x224c wordp=0x4044 params=2 returns=0
// ================================================

void DMIN() // DMIN
{
  _2OVER(); // 2OVER
  _2OVER(); // 2OVER
  D_gt_(); // D>
  if (Pop() != 0)
  {
    _2SWAP(); // 2SWAP
  }
  Pop(); Pop(); // 2DROP
}


// ================================================
// 0x4054: WORD 'DMAX' codep=0x224c wordp=0x405d params=2 returns=0
// ================================================

void DMAX() // DMAX
{
  _2OVER(); // 2OVER
  _2OVER(); // 2OVER
  D_st_(); // D<
  if (Pop() != 0)
  {
    _2SWAP(); // 2SWAP
  }
  Pop(); Pop(); // 2DROP
}


// ================================================
// 0x406d: WORD 'MYSELF' codep=0x224c wordp=0x4078
// ================================================

void MYSELF() // MYSELF
{
  LATEST(); // LATEST
  PFA(); // PFA
  CFA(); // CFA
  Push(Read16(regsp)); // DUP
  Push(Read16(Pop())==0x3cac?1:0); //  @ 0x3cac =
  if (Pop() != 0)
  {
    Push(Read16(Pop() + 2)); //  2+ @
  }
  _co_(); // ,
}


// ================================================
// 0x4094: WORD '>TIB' codep=0x224c wordp=0x409d params=2 returns=0
// ================================================

void _gt_TIB() // >TIB
{
  Push(Read16(user_TIB)); // TIB @
  _2DUP(); // 2DUP
  Push(Pop() + Pop()); // +
  OFF_1(); // OFF_1
  SWAP(); // SWAP
  CMOVE_1(); // CMOVE_1
  Push(user__gt_IN); // >IN
  OFF_1(); // OFF_1
}


// ================================================
// 0x40b1: WORD 'DOSPARM' codep=0x224c wordp=0x40bd
// ================================================

void DOSPARM() // DOSPARM
{
  Push(0x0080);
  COUNT(); // COUNT
  _gt_TIB(); // >TIB
  Exec("INTERPRET"); // call of word 0x1dd8 '(INTERPRET)'
}


// ================================================
// 0x40c9: WORD 'MAXDRV' codep=0x1d29 wordp=0x40d4
// ================================================
// 0x40d4: db 0x05 0x00 '  '

// ================================================
// 0x40d6: WORD '?#DETTE' codep=0x224c wordp=0x40e2 params=0 returns=2
// ================================================

void Is_n_DETTE() // ?#DETTE
{
  Push(0x0011);
  INTERRUPT(); // INTERRUPT
  Push(Read16(Read16(cc_AX)) & 0x00c0); // AX @ 0x00c0 AND
  Push(0x0040);
  _slash_(); // /
  Push(Pop() + 1); //  1+
}


// ================================================
// 0x40fc: WORD 'XFCB' codep=0x224c wordp=0x4105 params=0 returns=1
// ================================================

void XFCB() // XFCB
{
  Push(Read16(pp__i_FCB) - 7); // 'FCB @ 7 -
}


// ================================================
// 0x410f: WORD 'DRV' codep=0x224c wordp=0x4117 params=0 returns=1
// ================================================

void DRV() // DRV
{
  Push(Read16(pp__i_FCB)); // 'FCB @
}


// ================================================
// 0x411d: WORD 'NAM' codep=0x224c wordp=0x4125 params=0 returns=1
// ================================================

void NAM() // NAM
{
  Push(Read16(pp__i_FCB) + 1); // 'FCB @ 1+
}


// ================================================
// 0x412d: WORD 'TYP' codep=0x224c wordp=0x4135 params=0 returns=1
// ================================================

void TYP() // TYP
{
  Push(Read16(pp__i_FCB) + 9); // 'FCB @ 9 +
}


// ================================================
// 0x4141: WORD 'CURBLK' codep=0x224c wordp=0x414c params=0 returns=1
// ================================================

void CURBLK() // CURBLK
{
  Push(Read16(pp__i_FCB) + 0x000c); // 'FCB @ 0x000c +
}


// ================================================
// 0x4158: WORD 'RECSIZE' codep=0x224c wordp=0x4164 params=0 returns=1
// ================================================

void RECSIZE() // RECSIZE
{
  Push(Read16(pp__i_FCB) + 0x000e); // 'FCB @ 0x000e +
}


// ================================================
// 0x4170: WORD 'FILESIZ' codep=0x224c wordp=0x417c params=0 returns=1
// ================================================

void FILESIZ() // FILESIZ
{
  Push(Read16(pp__i_FCB) + 0x0010); // 'FCB @ 0x0010 +
}


// ================================================
// 0x4188: WORD 'CURREC' codep=0x224c wordp=0x4193 params=0 returns=1
// ================================================

void CURREC() // CURREC
{
  Push(Read16(pp__i_FCB) + 0x0020); // 'FCB @ 0x0020 +
}


// ================================================
// 0x419f: WORD 'RELREC' codep=0x224c wordp=0x41aa params=0 returns=1
// ================================================

void RELREC() // RELREC
{
  Push(Read16(pp__i_FCB) + 0x0021); // 'FCB @ 0x0021 +
}


// ================================================
// 0x41b6: WORD 'CLRFCB' codep=0x224c wordp=0x41c1
// ================================================

void CLRFCB() // CLRFCB
{
  XFCB(); // XFCB
  Push(0x002c);
  Push(0);
  FILL_1(); // FILL_1
  NAM(); // NAM
  Push(0x000b);
  Push(Read16(cc_BL)); // BL
  FILL_1(); // FILL_1
  Push(0);
  XFCB(); // XFCB
  C_ex__1(); // C!_1
}


// ================================================
// 0x41dd: WORD 'SYSTEM' codep=0x224c wordp=0x41e8 params=0 returns=0
// ================================================

void SYSTEM() // SYSTEM
{
  Push(0x2d97); // 'FCB'
  Push(Pop() + 7); //  7 +
  Push(pp__i_FCB); // 'FCB
  Store(); // !
}


// ================================================
// 0x41f6: WORD 'SYSUTIL' codep=0x224c wordp=0x4202 params=0 returns=0
// ================================================

void SYSUTIL() // SYSUTIL
{
  Push(0x2d97); // 'FCB'
  Push(Pop() + 0x0033); //  0x0033 +
  Push(pp__i_FCB); // 'FCB
  Store(); // !
}


// ================================================
// 0x4212: WORD 'DOSCALL' codep=0x224c wordp=0x421e
// ================================================

void DOSCALL() // DOSCALL
{
  Push(Read16(cc_AX) + 1); // AX 1+
  C_ex__1(); // C!_1
  Push(0x0021);
  INTERRUPT(); // INTERRUPT
}


// ================================================
// 0x422c: WORD '>FCB' codep=0x224c wordp=0x4235
// ================================================

void _gt_FCB() // >FCB
{
  DRV(); // DRV
  OFF_1(); // OFF_1
  NAM(); // NAM
  Push(0x000b);
  Push(Read16(cc_BL)); // BL
  FILL_1(); // FILL_1
  Push(Read16(cc_SI)); // SI
  Store(); // !
  Push(Read16(pp__i_FCB)); // 'FCB @
  Push(Read16(cc_DI)); // DI
  Store(); // !
  _ro_CS_ask__rc_(); // (CS?)
  Push(Read16(regsp)); // DUP
  Push(Read16(cc_DS)); // DS
  Store(); // !
  Push(Read16(cc_ES)); // ES
  Store(); // !
  Push(5);
  Push(Read16(cc_AX)); // AX
  Store(); // !
  Push(0x0029);
  DOSCALL(); // DOSCALL
}


// ================================================
// 0x4269: WORD 'SETFCB' codep=0x224c wordp=0x4274
// ================================================

void SETFCB() // SETFCB
{
  CLRFCB(); // CLRFCB
  Push(Read16(cc_BL)); // BL
  Exec("WORD"); // call of word 0x1f06 '(WORD)'
  Push(Pop() + 1); //  1+
  _gt_FCB(); // >FCB
  Push(((Read16(Read16(cc_AX))&0xFF)==0x00ff?1:0) | ((Read16(Read16(Read16(cc_DI)) + 1)&0xFF)==Read16(cc_BL)?1:0)); // AX C@ 0x00ff = DI @ 1+ C@ BL = OR
}


// ================================================
// 0x4298: WORD 'INIT' codep=0x224c wordp=0x42a1
// ================================================

void INIT() // INIT
{
  Push(0x000d);
  DOSCALL(); // DOSCALL
}


// ================================================
// 0x42a9: WORD 'FCBCALL' codep=0x224c wordp=0x42b5
// ================================================

void FCBCALL() // FCBCALL
{
  Push(Read16(pp__i_FCB)); // 'FCB @
  Push(Read16(cc_DX)); // DX
  Store(); // !
  _ro_CS_ask__rc_(); // (CS?)
  Push(Read16(cc_DS)); // DS
  Store(); // !
  DOSCALL(); // DOSCALL
  Push(Read16(Read16(cc_AX))&0xFF); // AX C@
}


// ================================================
// 0x42cb: WORD 'OPEN' codep=0x224c wordp=0x42d4
// ================================================

void OPEN() // OPEN
{
  Push(0x000f);
  FCBCALL(); // FCBCALL
}


// ================================================
// 0x42dc: WORD 'CLOSE' codep=0x224c wordp=0x42e6
// ================================================

void CLOSE() // CLOSE
{
  Push(0x0010);
  FCBCALL(); // FCBCALL
}


// ================================================
// 0x42ee: WORD 'SEARCH1' codep=0x224c wordp=0x42fa
// ================================================

void SEARCH1() // SEARCH1
{
  Push(0x0011);
  FCBCALL(); // FCBCALL
}


// ================================================
// 0x4302: WORD 'READNEX' codep=0x224c wordp=0x430e
// ================================================

void READNEX() // READNEX
{
  Push(0x0014);
  FCBCALL(); // FCBCALL
}


// ================================================
// 0x4316: WORD 'WRITENE' codep=0x224c wordp=0x4322
// ================================================

void WRITENE() // WRITENE
{
  Push(0x0015);
  FCBCALL(); // FCBCALL
}


// ================================================
// 0x432a: WORD 'MAKE' codep=0x224c wordp=0x4333
// ================================================

void MAKE() // MAKE
{
  Push(0x0016);
  FCBCALL(); // FCBCALL
}


// ================================================
// 0x433b: WORD 'DOS-DTA' codep=0x224c wordp=0x4347
// ================================================

void DOS_dash_DTA() // DOS-DTA
{
  Push(pp_DTA_1); // DTA_1
  _2_at_(); // 2@
  Push(Read16(cc_DX)); // DX
  Store(); // !
  Push(Read16(cc_DS)); // DS
  Store(); // !
  Push(0x001a);
  DOSCALL(); // DOSCALL
}


// ================================================
// 0x435b: WORD 'READ-RA' codep=0x224c wordp=0x4367
// ================================================

void READ_dash_RA() // READ-RA
{
  Push(0x0021);
  FCBCALL(); // FCBCALL
}


// ================================================
// 0x436f: WORD 'WRITE-R' codep=0x224c wordp=0x437b
// ================================================

void WRITE_dash_R() // WRITE-R
{
  Push(0x0022);
  FCBCALL(); // FCBCALL
}


// ================================================
// 0x4383: WORD 'W4385' codep=0x224c wordp=0x4385 params=1 returns=0
// ================================================

void W4385() // W4385
{
  ABORT("Bad name", 8);// (ABORT")
}


// ================================================
// 0x4392: WORD 'MAKE<NA' codep=0x224c wordp=0x439e
// ================================================

void MAKE_st_NA() // MAKE<NA
{
  SETFCB(); // SETFCB
  W4385(); // W4385
  MAKE(); // MAKE
  ABORT("Can't create.", 13);// (ABORT")
}


// ================================================
// 0x43b6: WORD 'OPEN<NA' codep=0x224c wordp=0x43c2
// ================================================

void OPEN_st_NA() // OPEN<NA
{
  SETFCB(); // SETFCB
  W4385(); // W4385
  OPEN(); // OPEN
  ABORT("Can't open.", 11);// (ABORT")
}


// ================================================
// 0x43d8: WORD '[FILE]' codep=0x224c wordp=0x43e3
// ================================================

void _bo_FILE_bc_() // [FILE]
{
  DOS_dash_DTA(); // DOS-DTA
  Push(Read16(pp_FILE)); // FILE @
  FCBPFAS(); // FCBPFAS
  Push(Read16(Pop())); //  @
  Push(pp__i_FCB); // 'FCB
  Store(); // !
  SWAP(); // SWAP
  RELREC(); // RELREC
  Store(); // !
  if (Pop() != 0)
  {
    READ_dash_RA(); // READ-RA
  } else
  {
    WRITE_dash_R(); // WRITE-R
  }
  Push(pp_DISK_dash_ERROR); // DISK-ERROR
  Store(); // !
}


// ================================================
// 0x4409: WORD 'SYSGEN' codep=0x224c wordp=0x4414
// ================================================

void SYSGEN() // SYSGEN
{
  Exec("CR"); // call of word 0x26ee '(CR)'
  FREEZE(); // FREEZE
  _ro_RESTORE_rc_(); // (RESTORE)
  SYSUTIL(); // SYSUTIL
  INIT(); // INIT
  MAKE_st_NA(); // MAKE<NA
  _ro_CS_ask__rc_(); // (CS?)
  Push(0x0100);
  Push(pp_DTA_1); // DTA_1
  _2_ex__1(); // 2!_1
  DOS_dash_DTA(); // DOS-DTA
  HERE(); // HERE
  Push(Pop() - 0x0100); //  0x0100 -
  RECSIZE(); // RECSIZE
  Store(); // !
  WRITENE(); // WRITENE
  _ro_SETUP_rc_(); // (SETUP)
  IsUNRAVEL(); // ?UNRAVEL
  CLOSE(); // CLOSE
  IsUNRAVEL(); // ?UNRAVEL
}


// ================================================
// 0x4444: WORD '4TH>DOS' codep=0x224c wordp=0x4450
// ================================================

void _4TH_gt_DOS() // 4TH>DOS
{
  unsigned short int i, imax;
  SYSUTIL(); // SYSUTIL
  MAKE_st_NA(); // MAKE<NA
  Push(0x0400);
  RECSIZE(); // RECSIZE
  Store(); // !
  Push(Pop() + 1); //  1+
  SWAP(); // SWAP

  i = Pop();
  imax = Pop();
  do // (DO)
  {
    _ro_CS_ask__rc_(); // (CS?)
    Push(i); // I
    BLOCK_1(); // BLOCK_1
    Push(pp_DTA_1); // DTA_1
    _2_ex__1(); // 2!_1
    DOS_dash_DTA(); // DOS-DTA
    SYSUTIL(); // SYSUTIL
    WRITENE(); // WRITENE
    IsUNRAVEL(); // ?UNRAVEL
    i++;
  } while(i<imax); // (LOOP)

  CLOSE(); // CLOSE
  IsUNRAVEL(); // ?UNRAVEL
}


// ================================================
// 0x447e: WORD 'W4480' codep=0x224c wordp=0x4480
// ================================================

void W4480() // W4480
{
  unsigned short int a;
  Push(Read16(pp__i_FCB)); // 'FCB @
  a = Pop(); // >R
  Push(0);
  PAD(); // PAD
  C_ex__1(); // C!_1
  PAD(); // PAD
  Push(Pop() + 7); //  7 +
  Push(pp__i_FCB); // 'FCB
  Store(); // !
  CLRFCB(); // CLRFCB
  SET_STR_AS_PARAM("STAR2A  COM");
  NAM(); // NAM
  SWAP(); // SWAP
  CMOVE_1(); // CMOVE_1
  _ro_CS_ask__rc_(); // (CS?)
  PAD(); // PAD
  Push(Pop() + 0x0032); //  0x0032 +
  Push(pp_DTA_1); // DTA_1
  _2_ex__1(); // 2!_1
  DOS_dash_DTA(); // DOS-DTA
  SEARCH1(); // SEARCH1
  Push(0x0042);
  NAM(); // NAM
  Push(Pop() + 4); //  4 +
  C_ex__1(); // C!_1
  SEARCH1(); // SEARCH1
  Push(Pop() | Pop()); // OR
  Push(a); // R>
  Push(pp__i_FCB); // 'FCB
  Store(); // !
}


// ================================================
// 0x44d6: WORD 'W44D8' codep=0x224c wordp=0x44d8
// ================================================

void W44D8() // W44D8
{
  Push(Read16(cc_DX)); // DX
  Store(); // !
  Push(0x0036);
  DOSCALL(); // DOSCALL
  Push(Read16(Read16(cc_AX))); // AX @
  ABS(); // ABS
  Push(!(Pop()==1?1:0)); //  1 = NOT
}


// ================================================
// 0x44f0: WORD 'W44F2' codep=0x224c wordp=0x44f2
// ================================================

void W44F2() // W44F2
{
  Push(0x0019);
  DOSCALL(); // DOSCALL
  Push((Read16(Read16(cc_AX))&0xFF) + 1); // AX C@ 1+
}


// ================================================
// 0x4500: WORD 'SETMAXD' codep=0x224c wordp=0x450c
// ================================================

void SETMAXD() // SETMAXD
{
  unsigned short int i, imax;
  Is_n_DETTE(); // ?#DETTE
  Push(2);
  MAX(); // MAX
  W44F2(); // W44F2
  MAX(); // MAX
  Push(pp_MAXDRV); // MAXDRV
  Store(); // !
  W4480(); // W4480
  if (Pop() == 0) return;

  i = Read16(pp_MAXDRV) + 1; // MAXDRV @ 1+
  imax = 7;
  do // (DO)
  {
    Push(i); // I
    W44D8(); // W44D8
    if (Pop() != 0)
    {
      Push(i); // I
      Push(pp_MAXDRV); // MAXDRV
      Store(); // !
    }
    i++;
  } while(i<imax); // (LOOP)

  Push(Read16(pp_MAXDRV)==2?1:0); // MAXDRV @ 2 =
  Is_n_DETTE(); // ?#DETTE
  Push(Pop()==1?1:0); //  1 =
  Push(Pop() & Pop()); // AND
  if (Pop() == 0) return;
  Push(1);
  Push(pp_MAXDRV); // MAXDRV
  Store(); // !
}


// ================================================
// 0x4558: WORD 'W455A' codep=0x224c wordp=0x455a params=0 returns=0
// ================================================

void W455A() // W455A
{
  DRV(); // DRV
  Push((Read16(Pop())&0xFF)==0?1:0); //  C@ 0=
  if (Pop() == 0) return;
  W44F2(); // W44F2
  DRV(); // DRV
  C_ex__1(); // C!_1
}


// ================================================
// 0x456c: WORD '?1DRV' codep=0x224c wordp=0x4576
// ================================================

void Is1DRV() // ?1DRV
{
  Is_n_DETTE(); // ?#DETTE
  Push(Pop()==1?1:0); //  1 =
  Push(0x0019);
  DOSCALL(); // DOSCALL
  Push(Pop() & ((Read16(Read16(cc_AX))&0xFF)==0?1:0)); //  AX C@ 0= AND
}


// ================================================
// 0x458c: WORD 'W458E' codep=0x224c wordp=0x458e
// ================================================

void W458E() // W458E
{
  Push(Pop() + 1); //  1+
  Push(Read16(pp_MAXDRV)); // MAXDRV @
  OVER(); // OVER
  _st_(); // <
  Is1DRV(); // ?1DRV
  Push(Pop() | Pop()); // OR
  if (Pop() == 0) return;
  Pop(); // DROP
  Push(1);
}


// ================================================
// 0x45a6: WORD 'SMARTOP' codep=0x224c wordp=0x45b2
// ================================================

void SMARTOP() // SMARTOP
{
  unsigned short int a, b;
  OPEN(); // OPEN
  Push(Read16(regsp)); // DUP
  if (Pop() == 0) return;
  DRV(); // DRV
  Push((Read16(Pop())&0xFF)==0?1:0); //  C@ 0=
  if (Pop() == 0) return;
  a = Pop(); // >R
  W455A(); // W455A
  DRV(); // DRV
  Push(Read16(Pop())&0xFF); //  C@
  while(1)
  {
    DRV(); // DRV
    Push(Read16(Pop())&0xFF); //  C@
    W458E(); // W458E
    _2DUP(); // 2DUP
    Push((Pop()==Pop())?1:0); // =
    Push(!Pop() & a); //  NOT I AND
    if (Pop() == 0) break;

    DRV(); // DRV
    C_ex__1(); // C!_1
    OPEN(); // OPEN
    Push(a); // R>
    Pop(); // DROP
    b = Pop(); // >R
  }
  Pop(); Pop(); // 2DROP
  Push(a); // R>
  Push(Read16(regsp)); // DUP
  if (Pop() == 0) return;
  Push(0);
  DRV(); // DRV
  C_ex__1(); // C!_1
}


// ================================================
// 0x4602: WORD 'ASKMOUN' codep=0x1d29 wordp=0x460e
// ================================================
// 0x460e: db 0x3b 0xaa '; '

// ================================================
// 0x4610: WORD ''BYE' codep=0x1d29 wordp=0x4619
// ================================================
// 0x4619: db 0x5d 0x08 '] '

// ================================================
// 0x461b: WORD 'SKIPPED' codep=0x1d29 wordp=0x4627
// ================================================
// 0x4627: db 0x00 0x00 '  '

// ================================================
// 0x4629: WORD 'CANSKIP' codep=0x1d29 wordp=0x4635
// ================================================
// 0x4635: db 0x00 0x00 '  '

// ================================================
// 0x4637: WORD '<ASKMOU' codep=0x224c wordp=0x4643 params=0 returns=0
// ================================================

void _st_ASKMOU() // <ASKMOU
{
  Exec("CR"); // call of word 0x26ee '(CR)'
  PRINT("Place disk with", 15); // (.")
  Exec("CR"); // call of word 0x26ee '(CR)'
  NAM(); // NAM
  Push(8);
  _dash_TRAILING(); // -TRAILING
  Exec("TYPE"); // call of word 0x2690 '(TYPE)'
  PRINT(".", 1); // (.")
  TYP(); // TYP
  Push(3);
  Exec("TYPE"); // call of word 0x2690 '(TYPE)'
  PRINT(" into ", 6); // (.")
  DRV(); // DRV
  Push(Read16(Pop())&0xFF); //  C@
  if (Read16(regsp) != 0) Push(Read16(regsp)); // ?DUP
  if (Pop() != 0)
  {
    PRINT("drive ", 6); // (.")
    Push(Pop() + 0x0040); //  0x0040 +
    Exec("EMIT"); // call of word 0x2731 '(EMIT)'
  } else
  {
    PRINT("any drive", 9); // (.")
  }
  Exec("CR"); // call of word 0x26ee '(CR)'
  PRINT("and press any key.", 18); // (.")
  Exec("KEY_1"); // call of word 0x25d7 '(KEY)'
  Push((Pop()==0x001a?1:0) & Read16(pp_CANSKIP)); //  0x001a = CANSKIP @ AND
  if (Pop() == 0) return;
  Push(pp_SKIPPED); // SKIPPED
  ON_3(); // ON_3
}


// ================================================
// 0x46ce: WORD 'DOSMOUN' codep=0x224c wordp=0x46da
// ================================================

void DOSMOUN() // DOSMOUN
{
  unsigned short int a;
  Push(pp_SKIPPED); // SKIPPED
  OFF_1(); // OFF_1
  a = Pop(); // >R
  INIT(); // INIT
  while(1)
  {
    Push(a); // I
    FCBPFAS(); // FCBPFAS
    Push(Read16(Pop())); //  @
    Push(pp__i_FCB); // 'FCB
    Store(); // !
    SMARTOP(); // SMARTOP
    Push(Read16(regsp)); // DUP
    Push(Pop() & !Read16(pp_SKIPPED)); //  SKIPPED @ NOT AND
    if (Pop() == 0) break;

    Pop(); // DROP
    FLUSH_1(); // FLUSH_1
    INIT(); // INIT
    Push(a); // I
    FCBPFAS(); // FCBPFAS
    Push(Read16(Pop())); //  @
    Push(pp__i_FCB); // 'FCB
    Store(); // !
    Push(pp_ASKMOUN); // ASKMOUN
    GetEXECUTE(); // @EXECUTE
  }
  Push(0x0400);
  RECSIZE(); // RECSIZE
  Store(); // !
  FILESIZ(); // FILESIZ
  _2_at_(); // 2@
  SWAP(); // SWAP
  Push(0x0400);
  M_slash_(); // M/
  SWAP(); // SWAP
  Pop(); // DROP
  DRV(); // DRV
  Push(Read16(Pop())&0xFF); //  C@
  Push(a); // R>
  DRIVENUMBERS(); // DRIVENUMBERS
  C_ex__1(); // C!_1
}


// ================================================
// 0x4738: WORD 'DOSUNMO' codep=0x224c wordp=0x4744
// ================================================

void DOSUNMO() // DOSUNMO
{
  FCBPFAS(); // FCBPFAS
  Push(Read16(Pop())); //  @
  Push(pp__i_FCB); // 'FCB
  Store(); // !
  CLOSE(); // CLOSE
  Pop(); // DROP
  Push(0);
}


// ================================================
// 0x4754: WORD 'DR2' codep=0x224c wordp=0x475c
// ================================================

void DR2() // DR2
{
  Push(0x43e3); // '[FILE]'
  Push(0x02d0);
  SYSTEM(); // SYSTEM
  Push(Read16(pp__i_FCB)); // 'FCB @
  NAM(); // NAM
  DRV(); // DRV
  Push(Read16(Pop())&0xFF); //  C@
  Push(1);
  Push(Read16(cc_DOS_dash_FILE)); // DOS-FILE
  _st_MOUNT_gt_(); // <MOUNT>
  Pop(); // DROP
}


// ================================================
// 0x477a: WORD 'DR3' codep=0x224c wordp=0x4782
// ================================================

void DR3() // DR3
{
  Push(0x43e3); // '[FILE]'
  Push(0x05dc);
  SYSUTIL(); // SYSUTIL
  Push(Read16(pp__i_FCB)); // 'FCB @
  NAM(); // NAM
  DRV(); // DRV
  Push(Read16(Pop())&0xFF); //  C@
  Push(1);
  Push(Read16(cc_DOS_dash_FILE) + 1); // DOS-FILE 1+
  _st_MOUNT_gt_(); // <MOUNT>
  Pop(); // DROP
}


// ================================================
// 0x47a2: WORD 'USING' codep=0x224c wordp=0x47ac
// ================================================

void USING() // USING
{
  Push(Read16(cc_DOS_dash_FILE)); // DOS-FILE
  UNMOUNT(); // UNMOUNT
  SYSTEM(); // SYSTEM
  SETFCB(); // SETFCB
  W4385(); // W4385
  DR2(); // DR2
}


// ================================================
// 0x47ba: WORD 'SETBLOC' codep=0x224c wordp=0x47c6
// ================================================

void SETBLOC() // SETBLOC
{
  Push(Read16(cc_ES)); // ES
  Store(); // !
  Push(Read16(regsp)); // DUP
  Push(Read16(cc_BX)); // BX
  Store(); // !
  Push(0x004a);
  DOSCALL(); // DOSCALL
  Push(Read16(Read16(cc_PSW)) & 1); // PSW @ 1 AND
  if (Pop() != 0)
  {
    Pop(); // DROP
    Push(Read16(Read16(cc_BX))); // BX @
    Push(1);
  } else
  {
    Push(0);
  }
  _ro_CS_ask__rc_(); // (CS?)
  Push(Read16(cc_ES)); // ES
  Store(); // !
}


// ================================================
// 0x47f8: WORD 'SETSYSK' codep=0x224c wordp=0x4804
// ================================================

void SETSYSK() // SETSYSK
{
  Push(0x7fff);
  _ro_CS_ask__rc_(); // (CS?)
  SETBLOC(); // SETBLOC
  Pop(); // DROP
  Push(Read16(regsp)); // DUP
  _ro_CS_ask__rc_(); // (CS?)
  Push(Pop() + Pop()); // +
  Push(0);
  Push(0x0040);
  U_slash_MOD(); // U/MOD
  Push(pp_SYSK); // SYSK
  Store(); // !
  Pop(); // DROP
  _ro_CS_ask__rc_(); // (CS?)
  SETBLOC(); // SETBLOC
  Pop(); // DROP
  Push(0x2000);
  U_st_(); // U<
  if (Pop() == 0) return;
  PRINT("Out of memory", 13); // (.")
  Push(0x0bb8);
  MS(); // MS
  BYE_1(); // BYE_1
}


// ================================================
// 0x484c: WORD 'W484E' codep=0x1d29 wordp=0x484e
// ================================================
// 0x484e: db 0x90 0x8d '  '

// ================================================
// 0x4850: WORD 'W4852' codep=0x1d29 wordp=0x4852
// ================================================
// 0x4852: db 0x4e 0x95 'N '

// ================================================
// 0x4854: WORD 'W4856' codep=0x1d29 wordp=0x4856
// ================================================
// 0x4856: db 0x4e 0x95 0x00 0x00 'N   '

// ================================================
// 0x485a: WORD '?ERR' codep=0x224c wordp=0x4863 params=0 returns=1
// ================================================

void IsERR() // ?ERR
{
  Push(Read16(Read16(cc_PSW)) & 1); // PSW @ 1 AND
}


// ================================================
// 0x486d: WORD 'W486F' codep=0x224c wordp=0x486f
// ================================================

void W486F() // W486F
{
  Push(Read16(pp_W484E)); // W484E @
  Push(Read16(pp_W4852)); // W4852 @
  Push(pp_W4856); // W4856
  Store(); // !
  Push(pp_W4856 + 2); // W4856 2+
  OFF_1(); // OFF_1
  Push(Read16(cc_DX)); // DX
  Store(); // !
  Push(3);
  Push(Read16(cc_AX)); // AX
  C_ex__1(); // C!_1
  _ro_CS_ask__rc_(); // (CS?)
  Push(Read16(cc_ES)); // ES
  Store(); // !
  Push(pp_W4856); // W4856
  Push(Read16(cc_BX)); // BX
  Store(); // !
  Push(0x004b);
  DOSCALL(); // DOSCALL
}


// ================================================
// 0x48a1: WORD '[LDS]' codep=0x224c wordp=0x48ab
// ================================================

void _bo_LDS_bc_() // [LDS]
{
  unsigned short int a, b;
  W486F(); // W486F
  IsERR(); // ?ERR
  Push(Read16(regsp)); // DUP
  if (Pop() == 0) return;
  DRV(); // DRV
  Push((Read16(Pop())&0xFF)==0?1:0); //  C@ 0=
  if (Pop() == 0) return;
  a = Pop(); // >R
  W455A(); // W455A
  DRV(); // DRV
  Push(Read16(Pop())&0xFF); //  C@
  while(1)
  {
    DRV(); // DRV
    Push(Read16(Pop())&0xFF); //  C@
    W458E(); // W458E
    _2DUP(); // 2DUP
    Push((Pop()==Pop())?1:0); // =
    Push(!Pop() & a); //  NOT I AND
    if (Pop() == 0) break;

    DRV(); // DRV
    C_ex__1(); // C!_1
    W486F(); // W486F
    IsERR(); // ?ERR
    Push(a); // R>
    Pop(); // DROP
    b = Pop(); // >R
  }
  Pop(); Pop(); // 2DROP
  Push(a); // R>
  Push(Read16(regsp)); // DUP
  if (Pop() == 0) return;
  Push(0);
  DRV(); // DRV
  C_ex__1(); // C!_1
}


// ================================================
// 0x48ff: WORD 'W4901' codep=0x1d29 wordp=0x4901
// ================================================
// 0x4901: db 0x00 0x52 0x32 0x20 0x20 0x20 0x20 0x20 0x20 0x4f 0x56 0x4c 0x00 0x00 0x00 0x00 0x41 ' R2      OVL    A'

// ================================================
// 0x4912: WORD '(LDS)' codep=0x224c wordp=0x491c
// ================================================

void _ro_LDS_rc_() // (LDS)
{
  unsigned short int a;
  Push(Read16(pp__i_FCB)); // 'FCB @
  a = Pop(); // >R
  Push(pp_W4901); // W4901
  Push(pp__i_FCB); // 'FCB
  Store(); // !
  OVER(); // OVER
  _gt_FCB(); // >FCB
  Push(pp_W4852); // W4852
  Store(); // !
  Push(pp_W484E); // W484E
  Store(); // !
  while(1)
  {
    _bo_LDS_bc_(); // [LDS]
    Push(Pop() & !Read16(pp_SKIPPED)); //  SKIPPED @ NOT AND
    if (Pop() == 0) break;

    Push(pp_ASKMOUN); // ASKMOUN
    GetEXECUTE(); // @EXECUTE
  }
  Push(a); // R>
  Push(pp__i_FCB); // 'FCB
  Store(); // !
}


// ================================================
// 0x4952: WORD 'W4954' codep=0x224c wordp=0x4954
// ================================================
// orphan

void W4954() // W4954
{
  SETMAXD(); // SETMAXD
  DOSPARM(); // DOSPARM
}


// ================================================
// 0x495a: WORD 'ON_1' codep=0x4961 wordp=0x4961
// ================================================
// 0x4961: pop    bx
// 0x4962: mov    ax,0001
// 0x4965: mov    [bx],ax
// 0x4967: lodsw
// 0x4968: mov    bx,ax
// 0x496a: jmp    word ptr [bx]

// ================================================
// 0x496c: WORD 'OFF_2' codep=0x4974 wordp=0x4974 params=1 returns=0
// ================================================
// 0x4974: pop    bx
// 0x4975: xor    ax,ax
// 0x4977: mov    [bx],ax
// 0x4979: lodsw
// 0x497a: mov    bx,ax
// 0x497c: jmp    word ptr [bx]

// ================================================
// 0x497e: WORD 'W4980' codep=0x4980 wordp=0x4980
// ================================================
// 0x4980: push   ds
// 0x4981: mov    ax,0040
// 0x4984: mov    ds,ax
// 0x4986: mov    bx,0017
// 0x4989: mov    [bx],al
// 0x498b: pop    ds
// 0x498c: lodsw
// 0x498d: mov    bx,ax
// 0x498f: jmp    word ptr [bx]

// ================================================
// 0x4991: WORD 'KEY_2' codep=0x224c wordp=0x4999
// ================================================

void KEY_2() // KEY_2
{
  W4980(); // W4980
  Exec("KEY_1"); // call of word 0x25d7 '(KEY)'
}


// ================================================
// 0x499f: WORD 'NULL' codep=0x3b74 wordp=0x49a8
// ================================================
// 0x49a8: dw 0x0000 0x0000

// ================================================
// 0x49ac: WORD 'ZZZ' codep=0x1d29 wordp=0x49b4
// ================================================
// 0x49b4: db 0x76 0x02 0x19 0x00 0x4f 0x54 0x20 0x20 0x6d 0x65 'v   OT  me'

// ================================================
// 0x49be: WORD '<LCMOVE' codep=0x49ca wordp=0x49ca params=5 returns=0
// ================================================
// 0x49ca: mov    bx,di
// 0x49cc: pop    cx
// 0x49cd: mov    ax,es
// 0x49cf: mov    dx,ds
// 0x49d1: pop    di
// 0x49d2: pop    es
// 0x49d3: mov    [49B4],si // ZZZ
// 0x49d7: pop    si
// 0x49d8: pop    ds
// 0x49d9: or     cx,cx
// 0x49db: jz     49E7
// 0x49dd: std    
// 0x49de: add    di,cx
// 0x49e0: dec    di
// 0x49e1: add    si,cx
// 0x49e3: dec    si
// 0x49e4: repz   
// 0x49e5: movsb
// 0x49e6: cld    
// 0x49e7: mov    es,ax
// 0x49e9: mov    ds,dx
// 0x49eb: mov    si,[49B4] // ZZZ
// 0x49ef: mov    di,bx
// 0x49f1: lodsw
// 0x49f2: mov    bx,ax
// 0x49f4: jmp    word ptr [bx]

// ================================================
// 0x49f6: WORD '2^N' codep=0x49fe wordp=0x49fe params=1 returns=1
// ================================================
// 0x49fe: pop    cx
// 0x49ff: xor    ax,ax
// 0x4a01: stc    
// 0x4a02: inc    cx
// 0x4a03: jcxz   4A07
// 0x4a05: rcl    ax,cl
// 0x4a07: push   ax
// 0x4a08: lodsw
// 0x4a09: mov    bx,ax
// 0x4a0b: jmp    word ptr [bx]

// ================================================
// 0x4a0d: WORD 'BIT' codep=0x224c wordp=0x4a15 params=1 returns=1
// ================================================

void BIT() // BIT
{
  Push(Pop() - 1); //  1-
  _2_h_N(); // 2^N
}


// ================================================
// 0x4a1b: WORD '16/' codep=0x4a23 wordp=0x4a23 params=1 returns=1
// ================================================
// 0x4a23: pop    ax
// 0x4a24: mov    cx,0004
// 0x4a27: shr    ax,cl
// 0x4a29: push   ax
// 0x4a2a: lodsw
// 0x4a2b: mov    bx,ax
// 0x4a2d: jmp    word ptr [bx]

// ================================================
// 0x4a2f: WORD '16*' codep=0x4a37 wordp=0x4a37 params=1 returns=1
// ================================================
// 0x4a37: pop    ax
// 0x4a38: mov    cx,0004
// 0x4a3b: shl    ax,cl
// 0x4a3d: push   ax
// 0x4a3e: lodsw
// 0x4a3f: mov    bx,ax
// 0x4a41: jmp    word ptr [bx]

// ================================================
// 0x4a43: WORD 'D16*' codep=0x4a4c wordp=0x4a4c params=2 returns=2
// ================================================
// 0x4a4c: pop    ax
// 0x4a4d: pop    dx
// 0x4a4e: mov    cx,0004
// 0x4a51: shl    dx,1
// 0x4a53: rcl    ax,1
// 0x4a55: loop   4A51
// 0x4a57: push   dx
// 0x4a58: push   ax
// 0x4a59: lodsw
// 0x4a5a: mov    bx,ax
// 0x4a5c: jmp    word ptr [bx]

// ================================================
// 0x4a5e: WORD '3*' codep=0x224c wordp=0x4a65 params=1 returns=1
// ================================================

void _3_star_() // 3*
{
  Push(Pop() * 3); //  3 *
}


// ================================================
// 0x4a6b: WORD '3+' codep=0x4a72 wordp=0x4a72 params=1 returns=1
// ================================================
// 0x4a72: pop    ax
// 0x4a73: add    ax,0003
// 0x4a76: push   ax
// 0x4a77: lodsw
// 0x4a78: mov    bx,ax
// 0x4a7a: jmp    word ptr [bx]

// ================================================
// 0x4a7c: WORD '1.5@' codep=0x4a85 wordp=0x4a85 params=1 returns=2
// ================================================
// 0x4a85: pop    bx
// 0x4a86: push   word ptr [bx]
// 0x4a88: xor    ax,ax
// 0x4a8a: mov    al,[bx+02]
// 0x4a8d: push   ax
// 0x4a8e: lodsw
// 0x4a8f: mov    bx,ax
// 0x4a91: jmp    word ptr [bx]

// ================================================
// 0x4a93: WORD '1.5!_1' codep=0x4a9c wordp=0x4a9c
// ================================================
// 0x4a9c: pop    bx
// 0x4a9d: pop    ax
// 0x4a9e: mov    [bx+02],al
// 0x4aa1: pop    word ptr [bx]
// 0x4aa3: lodsw
// 0x4aa4: mov    bx,ax
// 0x4aa6: jmp    word ptr [bx]

// ================================================
// 0x4aa8: WORD '@DS' codep=0x4ab0 wordp=0x4ab0 params=0 returns=1
// ================================================
// 0x4ab0: push   ds
// 0x4ab1: lodsw
// 0x4ab2: mov    bx,ax
// 0x4ab4: jmp    word ptr [bx]

// ================================================
// 0x4ab6: WORD 'W4AB8' codep=0x1d29 wordp=0x4ab8 params=0 returns=1
// ================================================
// 0x4ab8: push   cx
// 0x4ab9: es:    
// 0x4aba: mov    cl,[bx]
// 0x4abc: xchg   ax,bx
// 0x4abe: es:    
// 0x4abf: xchg   [bx],cl
// 0x4ac1: xchg   ax,bx
// 0x4ac3: es:    
// 0x4ac4: mov    [bx],cl
// 0x4ac6: pop    cx
// 0x4ac7: ret    


// ================================================
// 0x4ac8: WORD '+@' codep=0x224c wordp=0x4acf params=2 returns=1
// ================================================

void _plus__at_() // +@
{
  Push(Pop() + Pop()); // +
  Push(Read16(Pop())); //  @
}


// ================================================
// 0x4ad5: WORD '>FLAG' codep=0x224c wordp=0x4adf params=1 returns=1
// ================================================

void _gt_FLAG() // >FLAG
{
  Push((Pop()==0?1:0)==0?1:0); //  0= 0=
}


// ================================================
// 0x4ae5: WORD 'L+-@' codep=0x4aee wordp=0x4aee params=2 returns=1
// ================================================
// 0x4aee: mov    dx,ds
// 0x4af0: pop    bx
// 0x4af1: pop    ds
// 0x4af2: xor    ax,ax
// 0x4af4: mov    al,[bx]
// 0x4af6: cbw    
// 0x4af7: push   ax
// 0x4af8: mov    ds,dx
// 0x4afa: lodsw
// 0x4afb: mov    bx,ax
// 0x4afd: jmp    word ptr [bx]

// ================================================
// 0x4aff: WORD 'W4B01' codep=0x4b01 wordp=0x4b01
// ================================================
// 0x4b01: pop    bx
// 0x4b02: pop    ax
// 0x4b03: mov    cx,[bx]
// 0x4b05: add    bx,02
// 0x4b08: mov    dx,[bx]
// 0x4b0a: add    bx,02
// 0x4b0d: cmp    ax,[bx]
// 0x4b0f: jnz    4B19
// 0x4b11: add    bx,02
// 0x4b14: mov    dx,[bx]
// 0x4b16: mov    cx,0001
// 0x4b19: add    bx,04
// 0x4b1c: loop   4B0D
// 0x4b1e: push   dx
// 0x4b1f: lodsw
// 0x4b20: mov    bx,ax
// 0x4b22: jmp    word ptr [bx]

// ================================================
// 0x4b24: WORD 'CASE' codep=0x224c wordp=0x4b2d
// ================================================

void CASE() // CASE
{
  Exec("CREATE"); // call of word 0x1cbb '(CREATE)'
  HERE(); // HERE
  Push2Words("NULL");
  _co_(); // ,
  _co_(); // ,
  Push(0);
  CODE(); // (;CODE) inlined assembler code
// 0x4b3b: call   1649
  W4B01(); // W4B01
  EXECUTE(); // EXECUTE
}


// ================================================
// 0x4b44: WORD 'IS' codep=0x224c wordp=0x4b4b
// ================================================

void IS() // IS
{
  _co_(); // ,
  _i__2(); // '_2
  _co_(); // ,
  Push(Pop() + 1); //  1+
}


// ================================================
// 0x4b55: WORD 'OTHERS' codep=0x224c wordp=0x4b60
// ================================================

void OTHERS() // OTHERS
{
  _i__2(); // '_2
  Push(3);
  PICK(); // PICK
  Push(Pop() + 2); //  2+
  Store(); // !
  SWAP(); // SWAP
  Store(); // !
}


// ================================================
// 0x4b70: WORD 'CASE:' codep=0x224c wordp=0x4b7a
// ================================================

void CASE_c_() // CASE:
{
  Exec("CREATE"); // call of word 0x1cbb '(CREATE)'
  SMUDGE(); // SMUDGE
  _bc_(); // ]
  CODE(); // (;CODE) inlined assembler code
// 0x4b82: call   1649
  SWAP(); // SWAP
  Push(Read16(regsp)); // DUP
  Push(Pop() + Pop()); // +
  Push(Pop() + Pop()); // +
  Push(Read16(Pop())); //  @
  CFAEXEC(); // CFAEXEC
}


// ================================================
// 0x4b93: WORD 'SEED_3' codep=0x1d29 wordp=0x4b9c
// ================================================
// 0x4b9c: db 0xdf 0x97 '  '

// ================================================
// 0x4b9e: WORD 'FRND' codep=0x4ba7 wordp=0x4ba7 params=0 returns=1
// ================================================
// 0x4ba7: mov    ax,[4B9C] // SEED_3
// 0x4bab: mov    cx,7ABD
// 0x4bae: imul   cx
// 0x4bb0: add    ax,1B0F
// 0x4bb3: mov    [4B9C],ax // SEED_3
// 0x4bb7: push   ax
// 0x4bb8: lodsw
// 0x4bb9: mov    bx,ax
// 0x4bbb: jmp    word ptr [bx]

// ================================================
// 0x4bbd: WORD 'RRND' codep=0x224c wordp=0x4bc6 params=2 returns=1
// ================================================

void RRND() // RRND
{
  OVER(); // OVER
  _dash_(); // -
  FRND(); // FRND
  U_star_(); // U*
  SWAP(); // SWAP
  Pop(); // DROP
  Push(Pop() + Pop()); // +
}


// ================================================
// 0x4bd6: WORD '+BIT' codep=0x4bdf wordp=0x4bdf params=1 returns=1
// ================================================
// 0x4bdf: xor    dx,dx
// 0x4be1: pop    ax
// 0x4be2: mov    cx,0010
// 0x4be5: shr    ax,1
// 0x4be7: jae    4BEA
// 0x4be9: inc    dx
// 0x4bea: loop   4BE5
// 0x4bec: push   dx
// 0x4bed: lodsw
// 0x4bee: mov    bx,ax
// 0x4bf0: jmp    word ptr [bx]

// ================================================
// 0x4bf2: WORD 'D2*' codep=0x4bf4 wordp=0x4bf4 params=2 returns=2
// ================================================
// 0x4bf4: pop    ax
// 0x4bf5: pop    cx
// 0x4bf6: shl    cx,1
// 0x4bf8: rcl    ax,1
// 0x4bfa: push   cx
// 0x4bfb: push   ax
// 0x4bfc: lodsw
// 0x4bfd: mov    bx,ax
// 0x4bff: jmp    word ptr [bx]

// ================================================
// 0x4c01: WORD 'EASY-BITS' codep=0x4c03 wordp=0x4c03 params=4 returns=3
// ================================================
// 0x4c03: pop    cx
// 0x4c04: pop    ax
// 0x4c05: pop    dx
// 0x4c06: pop    bx
// 0x4c07: shl    bx,1
// 0x4c09: rcl    dx,1
// 0x4c0b: shl    bx,1
// 0x4c0d: rcl    dx,1
// 0x4c0f: sub    dx,ax
// 0x4c11: jns    4C1A
// 0x4c13: add    dx,ax
// 0x4c15: shl    ax,1
// 0x4c17: dec    ax
// 0x4c18: jmp    4C1E
// 0x4c1a: inc    ax
// 0x4c1b: shl    ax,1
// 0x4c1d: inc    ax
// 0x4c1e: loop   4C07
// 0x4c20: push   bx
// 0x4c21: push   dx
// 0x4c22: push   ax
// 0x4c23: lodsw
// 0x4c24: mov    bx,ax
// 0x4c26: jmp    word ptr [bx]

// ================================================
// 0x4c28: WORD 'W4C2A' codep=0x224c wordp=0x4c2a params=3 returns=3
// ================================================

void W4C2A() // W4C2A
{
  unsigned short int a;
  a = Pop(); // >R
  D2_star_(); // D2*
  Push(Read16(regsp)); // DUP
  _0_st_(); // 0<
  if (Pop() != 0)
  {
    D2_star_(); // D2*
    Push(Read16(a)); // R@
    _dash_(); // -
    Push(a + 1); // R> 1+
    return;
  }
  D2_star_(); // D2*
  Push(Read16(a)); // R@
  _2DUP(); // 2DUP
  U_st_(); // U<
  if (Pop() != 0)
  {
    Pop(); // DROP
    Push(a - 1); // R> 1-
    return;
  }
  _dash_(); // -
  Push(a + 1); // R> 1+
}


// ================================================
// 0x4c62: WORD 'W4C64' codep=0x224c wordp=0x4c64 params=3 returns=1
// ================================================

void W4C64() // W4C64
{
  unsigned short int a;
  a = Pop(); // >R
  Push(Read16(regsp)); // DUP
  _0_st_(); // 0<
  if (Pop() != 0)
  {
    Pop(); Pop(); // 2DROP
    Push(a + 1); // R> 1+
    return;
  }
  D2_star_(); // D2*
  Push(0x8000);
  Push(Read16(a)); // R@
  DU_st_(); // DU<
  Push((Pop()==0?1:0) + a); //  0= R> +
}


// ================================================
// 0x4c8a: WORD 'SQRT' codep=0x224c wordp=0x4c93 params=2 returns=1
// ================================================

void SQRT() // SQRT
{
  Push(0);
  Push(1);
  Push(8);
  EASY_dash_BITS(); // EASY-BITS
  ROT(); // ROT
  Pop(); // DROP
  Push(6);
  EASY_dash_BITS(); // EASY-BITS
  W4C2A(); // W4C2A
  W4C64(); // W4C64
}


// ================================================
// 0x4ca9: WORD '+-@' codep=0x4cb1 wordp=0x4cb1 params=1 returns=1
// ================================================
// 0x4cb1: pop    bx
// 0x4cb2: mov    al,[bx]
// 0x4cb4: cbw    
// 0x4cb5: push   ax
// 0x4cb6: lodsw
// 0x4cb7: mov    bx,ax
// 0x4cb9: jmp    word ptr [bx]

// ================================================
// 0x4cbb: WORD 'L1.5@' codep=0x4cc5 wordp=0x4cc5 params=2 returns=2
// ================================================
// 0x4cc5: pop    bx
// 0x4cc6: pop    cx
// 0x4cc7: mov    ax,ds
// 0x4cc9: mov    ds,cx
// 0x4ccb: push   word ptr [bx]
// 0x4ccd: xor    cx,cx
// 0x4ccf: mov    cl,[bx+02]
// 0x4cd2: push   cx
// 0x4cd3: mov    ds,ax
// 0x4cd5: lodsw
// 0x4cd6: mov    bx,ax
// 0x4cd8: jmp    word ptr [bx]

// ================================================
// 0x4cda: WORD '<C!>' codep=0x0c60 wordp=0x4ce3
// ================================================

// ================================================
// 0x4ce3: WORD '<!>' codep=0x0be1 wordp=0x4ceb
// ================================================

// ================================================
// 0x4ceb: WORD '<1.5!>' codep=0x4a9c wordp=0x4cf6
// ================================================

// ================================================
// 0x4cf6: WORD '<+!>' codep=0x0f85 wordp=0x4cff
// ================================================

// ================================================
// 0x4cff: WORD '<D!>' codep=0x0c24 wordp=0x4d08
// ================================================

// ================================================
// 0x4d08: WORD '<OFF>' codep=0x3af8 wordp=0x4d12
// ================================================
// 0x4d12: db 0x72 0x49 'rI'

// ================================================
// 0x4d14: WORD '<ON>' codep=0x3af8 wordp=0x4d1d
// ================================================
// 0x4d1d: db 0x5f 0x49 '_I'

// ================================================
// 0x4d1f: WORD '<BLOCK>' codep=0x3af8 wordp=0x4d2b
// ================================================
// 0x4d2b: db 0x5d 0x38 ']8'

// ================================================
// 0x4d2d: WORD 'SPHEREW' codep=0x1d29 wordp=0x4d39
// ================================================
// 0x4d39: db 0x50 0x20 'P '

// ================================================
// 0x4d3b: WORD 'SIGNEXT' codep=0x1d29 wordp=0x4d47
// ================================================
// 0x4d47: db 0x44 0x20 'D '

// ================================================
// 0x4d49: WORD 'W4D4B' codep=0x1d29 wordp=0x4d4b
// ================================================
// 0x4d4b: db 0x3a 0x20 ': '

// ================================================
// 0x4d4d: WORD 'W4D4F' codep=0x1d29 wordp=0x4d4f
// ================================================
// 0x4d4f: db 0x3a 0x20 ': '

// ================================================
// 0x4d51: WORD 'W4D53' codep=0x1d29 wordp=0x4d53
// ================================================
// 0x4d53: db 0x3a 0x20 ': '

// ================================================
// 0x4d55: WORD ''ARRAY' codep=0x1d29 wordp=0x4d60
// ================================================
// 0x4d60: db 0x82 0x6a ' j'

// ================================================
// 0x4d62: WORD 'ARRAYSE' codep=0x224c wordp=0x4d6e params=0 returns=1
// ================================================

void ARRAYSE() // ARRAYSE
{
  Push(Read16(pp__i_ARRAY)); // 'ARRAY @
  Push(6);
  _plus__at_(); // +@
}


// ================================================
// 0x4d78: WORD '#BYTES' codep=0x224c wordp=0x4d83 params=0 returns=1
// ================================================

void _n_BYTES() // #BYTES
{
  Push(Read16(pp__i_ARRAY)); // 'ARRAY @
  Push(4);
  _plus__at_(); // +@
}


// ================================================
// 0x4d8d: WORD '#COLZ' codep=0x224c wordp=0x4d97 params=0 returns=1
// ================================================

void _n_COLZ() // #COLZ
{
  Push(Read16(Read16(pp__i_ARRAY))); // 'ARRAY @ @
}


// ================================================
// 0x4d9f: WORD '#ROWZ' codep=0x224c wordp=0x4da9 params=0 returns=1
// ================================================

void _n_ROWZ() // #ROWZ
{
  Push(Read16(Read16(pp__i_ARRAY) + 2)); // 'ARRAY @ 2+ @
}


// ================================================
// 0x4db3: WORD 'ACELLAD' codep=0x224c wordp=0x4dbf
// ================================================

void ACELLAD() // ACELLAD
{
  Push(Read16(pp__i_ARRAY)); // 'ARRAY @
  Push(pp_W4D4B); // W4D4B
  GetEXECUTE(); // @EXECUTE
}


// ================================================
// 0x4dc9: WORD 'W4DCB' codep=0x4dcb wordp=0x4dcb
// ================================================
// 0x4dcb: pop    bx
// 0x4dcc: add    bx,06
// 0x4dcf: mov    ax,[bx]
// 0x4dd1: sub    bx,02
// 0x4dd4: mov    cx,[bx]
// 0x4dd6: mov    bx,cx
// 0x4dd8: pop    cx
// 0x4dd9: shl    cx,1
// 0x4ddb: add    bx,cx
// 0x4ddd: push   ds
// 0x4dde: mov    ds,ax
// 0x4de0: mov    cx,[bx]
// 0x4de2: pop    ds
// 0x4de3: pop    dx
// 0x4de4: add    cx,dx
// 0x4de6: push   ax
// 0x4de7: push   cx
// 0x4de8: lodsw
// 0x4de9: mov    bx,ax
// 0x4deb: jmp    word ptr [bx]

// ================================================
// 0x4ded: WORD 'A!' codep=0x224c wordp=0x4df4
// ================================================

void A_ex_() // A!
{
  Push(pp_W4D4F); // W4D4F
  GetEXECUTE(); // @EXECUTE
}


// ================================================
// 0x4dfa: WORD 'A@' codep=0x224c wordp=0x4e01
// ================================================

void A_at_() // A@
{
  Push(pp_W4D53); // W4D53
  GetEXECUTE(); // @EXECUTE
}


// ================================================
// 0x4e07: WORD '!OFFSETS' codep=0x4e13 wordp=0x4e13 params=1 returns=0
// ================================================
// 0x4e13: pop    bx
// 0x4e14: push   si
// 0x4e15: push   di
// 0x4e16: push   bp
// 0x4e17: push   es
// 0x4e18: mov    si,[bx]
// 0x4e1a: mov    cx,[bx+02]
// 0x4e1d: mov    bp,[bx+04]
// 0x4e20: push   word ptr [bx+06]
// 0x4e23: pop    es
// 0x4e24: mov    di,cx
// 0x4e26: shl    di,1
// 0x4e28: add    di,bp
// 0x4e2a: std    
// 0x4e2b: mov    ax,si
// 0x4e2d: mul    cx
// 0x4e2f: stosw
// 0x4e30: loop   4E2B
// 0x4e32: xor    ax,ax
// 0x4e34: stosw
// 0x4e35: cld    
// 0x4e36: pop    es
// 0x4e37: pop    bp
// 0x4e38: pop    di
// 0x4e39: pop    si
// 0x4e3a: lodsw
// 0x4e3b: mov    bx,ax
// 0x4e3d: jmp    word ptr [bx]

// ================================================
// 0x4e3f: WORD 'ARRAY' codep=0x224c wordp=0x4e49
// ================================================

void ARRAY() // ARRAY
{
  unsigned short int a, b;
  Exec("CREATE"); // call of word 0x1cbb '(CREATE)'
  HERE(); // HERE
  a = Pop(); // >R
  b = Pop(); // >R
  SWAP(); // SWAP
  _co_(); // ,
  _co_(); // ,
  Push(Read16(a) * Read16(a + 2)); // I' @ I' 2+ @ *
  _co_(); // ,
  Push(b); // R>
  _co_(); // ,
  Push(a); // R>
  Pop(); // DROP
  CODE(); // (;CODE) inlined assembler code
// 0x4e6f: call   1649
  W4DCB(); // W4DCB
}


// ================================================
// 0x4e76: WORD 'SETLARR' codep=0x224c wordp=0x4e82 params=1 returns=0
// ================================================

void SETLARR() // SETLARR
{
  Push(pp__i_ARRAY); // 'ARRAY
  Store(); // !
  Push(0x4dcb); // 'W4DCB'
  Push(pp_W4D4B); // W4D4B
  Store(); // !
  Push(0x2ee5); // 'LC!'
  Push(pp_W4D4F); // W4D4F
  Store(); // !
  Push(Read16(pp_SIGNEXT)); // SIGNEXT @
  if (Pop() != 0)
  {
    Push(0x4aee); // 'L+-@'
  } else
  {
    Push(0x2ecd); // 'LC@'
  }
  Push(pp_W4D53); // W4D53
  Store(); // !
}


// ================================================
// 0x4eb0: WORD 'XLL' codep=0x1d29 wordp=0x4eb8
// ================================================
// 0x4eb8: db 0x41 0x44 'AD'

// ================================================
// 0x4eba: WORD 'YLL' codep=0x1d29 wordp=0x4ec2
// ================================================
// 0x4ec2: db 0x72 0x65 're'

// ================================================
// 0x4ec4: WORD 'XUR' codep=0x1d29 wordp=0x4ecc
// ================================================
// 0x4ecc: db 0x72 0x65 're'

// ================================================
// 0x4ece: WORD 'YUR' codep=0x1d29 wordp=0x4ed6
// ================================================
// 0x4ed6: db 0x72 0x65 're'

// ================================================
// 0x4ed8: WORD 'FULLARR' codep=0x224c wordp=0x4ee4 params=0 returns=4
// ================================================

void FULLARR() // FULLARR
{
  Push(0);
  Push(0);
  _n_COLZ(); // #COLZ
  Push(Pop() - 1); //  1-
  _n_ROWZ(); // #ROWZ
  Push(Pop() - 1); //  1-
}


// ================================================
// 0x4ef2: WORD 'SETREGI' codep=0x224c wordp=0x4efe params=4 returns=0
// ================================================

void SETREGI() // SETREGI
{
  Push(pp_YUR); // YUR
  Store(); // !
  Push(pp_XUR); // XUR
  Store(); // !
  Push(pp_YLL); // YLL
  Store(); // !
  Push(pp_XLL); // XLL
  Store(); // !
}


// ================================================
// 0x4f10: WORD 'FILLREG' codep=0x224c wordp=0x4f1c
// ================================================

void FILLREG() // FILLREG
{
  unsigned short int i, imax, j, jmax;

  i = Read16(pp_YLL); // YLL @
  imax = Read16(pp_YUR) + 1; // YUR @ 1+
  do // (DO)
  {

    j = Read16(pp_XLL); // XLL @
    jmax = Read16(pp_XUR) + 1; // XUR @ 1+
    do // (DO)
    {
      Push(Read16(regsp)); // DUP
      Push(j); // I
      Push(i); // J
      ACELLAD(); // ACELLAD
      A_ex_(); // A!
      j++;
    } while(j<jmax); // (LOOP)

    i++;
  } while(i<imax); // (LOOP)

  Pop(); // DROP
}


// ================================================
// 0x4f4a: WORD 'FONT#' codep=0x1d29 wordp=0x4f54
// ================================================
// 0x4f54: db 0x00 0x00 '  '

// ================================================
// 0x4f56: WORD 'TANDY' codep=0x1d29 wordp=0x4f60
// ================================================
// 0x4f60: db 0x10 0x00 '  '

// ================================================
// 0x4f62: WORD 'COLORT' codep=0x1d29 wordp=0x4f6d
// ================================================
// 0x4f6d: db 0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0a 0x0b 0x0c 0x0d 0x0e 0x0f 0x00 '                 '

// ================================================
// 0x4f7e: WORD 'I>C' codep=0x224c wordp=0x4f86 params=1 returns=1
// ================================================

void I_gt_C() // I>C
{
  Push(Read16(Pop() + pp_COLORT)&0xFF); //  COLORT + C@
}


// ================================================
// 0x4f8e: WORD 'W4F90' codep=0x224c wordp=0x4f90
// ================================================
// orphan

void W4F90() // W4F90
{
  Exec("CREATE"); // call of word 0x1cbb '(CREATE)'
  C_co_(); // C,
  CODE(); // (;CODE) inlined assembler code
// 0x4f96: call   1649
  Push(Read16(Pop())&0xFF); //  C@
  I_gt_C(); // I>C
}


// ================================================
// 0x4f9f: WORD 'BLACK' codep=0x4f96 wordp=0x4fa9
// ================================================
Color BLACK = 0x00

// ================================================
// 0x4faa: WORD 'DK-BLUE' codep=0x4f96 wordp=0x4fb6
// ================================================
Color DK_dash_BLUE = 0x01

// ================================================
// 0x4fb7: WORD 'DK-GREE' codep=0x4f96 wordp=0x4fc3
// ================================================
Color DK_dash_GREE = 0x02

// ================================================
// 0x4fc4: WORD 'GREEN' codep=0x4f96 wordp=0x4fce
// ================================================
Color GREEN = 0x03

// ================================================
// 0x4fcf: WORD 'RED' codep=0x4f96 wordp=0x4fd7
// ================================================
Color RED = 0x04

// ================================================
// 0x4fd8: WORD 'VIOLET' codep=0x4f96 wordp=0x4fe3
// ================================================
Color VIOLET = 0x05

// ================================================
// 0x4fe4: WORD 'BROWN' codep=0x4f96 wordp=0x4fee
// ================================================
Color BROWN = 0x06

// ================================================
// 0x4fef: WORD 'GREY1' codep=0x4f96 wordp=0x4ff9
// ================================================
Color GREY1 = 0x07

// ================================================
// 0x4ffa: WORD 'GREY2' codep=0x4f96 wordp=0x5004
// ================================================
Color GREY2 = 0x08

// ================================================
// 0x5005: WORD 'BLUE' codep=0x4f96 wordp=0x500e
// ================================================
Color BLUE = 0x09

// ================================================
// 0x500f: WORD 'LT-GREE' codep=0x4f96 wordp=0x501b
// ================================================
Color LT_dash_GREE = 0x0a

// ================================================
// 0x501c: WORD 'LT-BLUE' codep=0x4f96 wordp=0x5028
// ================================================
Color LT_dash_BLUE = 0x0b

// ================================================
// 0x5029: WORD 'PINK' codep=0x4f96 wordp=0x5032
// ================================================
Color PINK = 0x0c

// ================================================
// 0x5033: WORD 'ORANGE' codep=0x4f96 wordp=0x503e
// ================================================
Color ORANGE = 0x0d

// ================================================
// 0x503f: WORD 'YELLOW' codep=0x4f96 wordp=0x504a
// ================================================
Color YELLOW = 0x0e

// ================================================
// 0x504b: WORD 'WHITE' codep=0x4f96 wordp=0x5055
// ================================================
Color WHITE = 0x0f

// ================================================
// 0x5056: WORD '?NID' codep=0x1d29 wordp=0x505f
// ================================================
// 0x505f: db 0x5d 0x02 '] '

// ================================================
// 0x5061: WORD 'BMAP' codep=0x1d29 wordp=0x506a
// ================================================
// 0x506a: db 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 '              '

// ================================================
// 0x5078: WORD 'W507A' codep=0x224c wordp=0x507a params=1 returns=1
// ================================================

void W507A() // W507A
{
  Push(Pop() + pp_BMAP); //  BMAP +
}


// ================================================
// 0x5080: WORD 'BMOFF' codep=0x224c wordp=0x508a params=0 returns=1
// ================================================

void BMOFF() // BMOFF
{
  Push(0);
  W507A(); // W507A
}


// ================================================
// 0x5090: WORD 'BMSEG' codep=0x224c wordp=0x509a params=0 returns=1
// ================================================

void BMSEG() // BMSEG
{
  Push(2);
  W507A(); // W507A
}


// ================================================
// 0x50a0: WORD 'BMWIDE' codep=0x224c wordp=0x50ab params=0 returns=1
// ================================================

void BMWIDE() // BMWIDE
{
  Push(4);
  W507A(); // W507A
}


// ================================================
// 0x50b1: WORD 'BMBYTES' codep=0x224c wordp=0x50bd params=0 returns=1
// ================================================

void BMBYTES() // BMBYTES
{
  Push(0x000a);
  W507A(); // W507A
}


// ================================================
// 0x50c5: WORD 'BMHIGH' codep=0x224c wordp=0x50d0 params=0 returns=1
// ================================================

void BMHIGH() // BMHIGH
{
  Push(0x000b);
  W507A(); // W507A
}


// ================================================
// 0x50d8: WORD 'BMPAL' codep=0x224c wordp=0x50e2 params=0 returns=1
// ================================================

void BMPAL() // BMPAL
{
  Push(0x000d);
  W507A(); // W507A
}


// ================================================
// 0x50ea: WORD 'IHEADLE' codep=0x2214 wordp=0x50f6
// ================================================
// 0x50f6: dw 0x000b

// ================================================
// 0x50f8: WORD 'W50FA' codep=0x2214 wordp=0x50fa
// ================================================
// 0x50fa: dw 0x658a

// ================================================
// 0x50fc: WORD 'W50FE' codep=0x2214 wordp=0x50fe
// ================================================
// 0x50fe: dw 0x65d1

// ================================================
// 0x5100: WORD 'W5102' codep=0x2214 wordp=0x5102
// ================================================
// 0x5102: dw 0xf560

// ================================================
// 0x5104: WORD '*MAPSCA' codep=0x2214 wordp=0x5110
// ================================================
// 0x5110: dw 0x0008

// ================================================
// 0x5112: WORD 'SIGBLK' codep=0x2214 wordp=0x511d
// ================================================
// 0x511d: dw 0x0003

// ================================================
// 0x511f: WORD 'ALOVSA' codep=0x2214 wordp=0x512a
// ================================================
// 0x512a: dw 0x0000

// ================================================
// 0x512c: WORD 'AHIVSA' codep=0x2214 wordp=0x5137
// ================================================
// 0x5137: dw 0x3e7f

// ================================================
// 0x5139: WORD 'BLOVSA' codep=0x2214 wordp=0x5144
// ================================================
// 0x5144: dw 0x3e80

// ================================================
// 0x5146: WORD 'BHIVSA' codep=0x2214 wordp=0x5151
// ================================================
// 0x5151: dw 0x96ff

// ================================================
// 0x5153: WORD ''OVBACK' codep=0x2214 wordp=0x515f
// ================================================
// 0x515f: dw 0xd284

// ================================================
// 0x5161: WORD 'MUSSEG' codep=0x2214 wordp=0x516c
// ================================================
// 0x516c: dw 0x84c8

// ================================================
// 0x516e: WORD 'WIN' codep=0x1d29 wordp=0x5176
// ================================================
// 0x5176: db 0x00 0x00 '  '

// ================================================
// 0x5178: WORD '?FIRED-' codep=0x2214 wordp=0x5184
// ================================================
// 0x5184: dw 0x0000

// ================================================
// 0x5186: WORD '?A-SHIE' codep=0x2214 wordp=0x5192
// ================================================
// 0x5192: dw 0x0000

// ================================================
// 0x5194: WORD '?A-WEAP' codep=0x2214 wordp=0x51a0
// ================================================
// 0x51a0: dw 0x0000

// ================================================
// 0x51a2: WORD '16K' codep=0x2214 wordp=0x51aa
// ================================================
// 0x51aa: dw 0x4000

// ================================================
// 0x51ac: WORD 'DBUF-SI' codep=0x3b74 wordp=0x51b8
// ================================================
// 0x51b8: dw 0x0001 0x0000

// ================================================
// 0x51bc: WORD 'TRUE' codep=0x2214 wordp=0x51c5
// ================================================
// 0x51c5: dw 0x0001

// ================================================
// 0x51c7: WORD 'FALSE' codep=0x2214 wordp=0x51d1
// ================================================
// 0x51d1: dw 0x0000

// ================================================
// 0x51d3: WORD '?UF' codep=0x1d29 wordp=0x51db
// ================================================
// 0x51db: db 0x00 0x00 '  '

// ================================================
// 0x51dd: WORD '?AF' codep=0x1d29 wordp=0x51e5
// ================================================
// 0x51e5: db 0x00 0x00 '  '

// ================================================
// 0x51e7: WORD '#SHOTS' codep=0x1d29 wordp=0x51f2
// ================================================
// 0x51f2: db 0x75 0x02 'u '

// ================================================
// 0x51f4: WORD ''ASYS' codep=0x1d29 wordp=0x51fe
// ================================================
// 0x51fe: db 0xda 0xca '  '

// ================================================
// 0x5200: WORD 'P/B' codep=0x1d29 wordp=0x5208
// ================================================
// 0x5208: db 0x02 0x00 '  '

// ================================================
// 0x520a: WORD ''BOSS' codep=0x1d29 wordp=0x5214
// ================================================
// 0x5214: db 0xb8 0xd5 '  '

// ================================================
// 0x5216: WORD '%TALK' codep=0x1d29 wordp=0x5220
// ================================================
// 0x5220: db 0xe8 0x29 ' )'

// ================================================
// 0x5222: WORD 'NULL-IC' codep=0x2214 wordp=0x522e
// ================================================
// 0x522e: dw 0x0032

// ================================================
// 0x5230: WORD 'SYS-ICO' codep=0x2214 wordp=0x523c
// ================================================
// 0x523c: dw 0x00fd

// ================================================
// 0x523e: WORD 'INVIS-I' codep=0x2214 wordp=0x524a
// ================================================
// 0x524a: dw 0x00fe

// ================================================
// 0x524c: WORD 'FLUX-IC' codep=0x2214 wordp=0x5258
// ================================================
// 0x5258: dw 0x00ff

// ================================================
// 0x525a: WORD 'DEAD-IC' codep=0x2214 wordp=0x5266
// ================================================
// 0x5266: dw 0x0002

// ================================================
// 0x5268: WORD 'DEFAULT' codep=0x2214 wordp=0x5274
// ================================================
// 0x5274: dw 0x0001

// ================================================
// 0x5276: WORD 'TEXTC/L' codep=0x2214 wordp=0x5282
// ================================================
// 0x5282: dw 0x0026

// ================================================
// 0x5284: WORD 'W5286' codep=0x2214 wordp=0x5286
// ================================================
// 0x5286: dw 0x02d0

// ================================================
// 0x5288: WORD 'FUEL/SE' codep=0x2214 wordp=0x5294
// ================================================
// 0x5294: dw 0x0005

// ================================================
// 0x5296: WORD 'POLYSEG' codep=0x2214 wordp=0x52a2
// ================================================
// 0x52a2: dw 0x9e43

// ================================================
// 0x52a4: WORD ''ANSYS' codep=0x2214 wordp=0x52af
// ================================================
// 0x52af: dw 0xd38a

// ================================================
// 0x52b1: WORD '?CALLED' codep=0x2214 wordp=0x52bd
// ================================================
// 0x52bd: dw 0x0000

// ================================================
// 0x52bf: WORD '0.' codep=0x3b74 wordp=0x52c6
// ================================================
// 0x52c6: dw 0x0000 0x0000

// ================================================
// 0x52ca: WORD 'VANEWSP' codep=0x3b74 wordp=0x52d6
// ================================================
// 0x52d6: dw 0x0000 0x1000

// ================================================
// 0x52da: WORD 'IROOT' codep=0x3b74 wordp=0x52e4
// ================================================
// 0x52e4: dw 0x0000 0x1006

// ================================================
// 0x52e8: WORD 'INACTIV' codep=0x3b74 wordp=0x52f4
// ================================================
// 0x52f4: dw 0x0000 0x1032

// ================================================
// 0x52f8: WORD 'FRAGMEN' codep=0x3b74 wordp=0x5304
// ================================================
// 0x5304: dw 0x0000 0x1027

// ================================================
// 0x5308: WORD '*SECS' codep=0x3b74 wordp=0x5312
// ================================================
// 0x5312: dw 0x0000 0x110e

// ================================================
// 0x5316: WORD '*ASSIGN' codep=0x3b74 wordp=0x5322
// ================================================
// 0x5322: dw 0x0002 0x2b25

// ================================================
// 0x5326: WORD '*SHIP' codep=0x3b74 wordp=0x5330
// ================================================
// 0x5330: dw 0x0002 0x2b7c

// ================================================
// 0x5334: WORD '*ASYS' codep=0x3b74 wordp=0x533e
// ================================================
// 0x533e: dw 0x0001 0xbfd1

// ================================================
// 0x5342: WORD '*ARTH' codep=0x3b74 wordp=0x534c
// ================================================
// 0x534c: dw 0x0001 0xbfe5

// ================================================
// 0x5350: WORD '*ARREST' codep=0x3b74 wordp=0x535c
// ================================================
// 0x535c: dw 0x0002 0x2766

// ================================================
// 0x5360: WORD '*HL' codep=0x3b74 wordp=0x5368
// ================================================
// 0x5368: dw 0x0001 0xd5a0

// ================================================
// 0x536c: WORD '*LH' codep=0x3b74 wordp=0x5374
// ================================================
// 0x5374: dw 0x0001 0xd99f

// ================================================
// 0x5378: WORD '*GD' codep=0x3b74 wordp=0x5380
// ================================================
// 0x5380: dw 0x0001 0xd02a

// ================================================
// 0x5384: WORD '*4SALE' codep=0x3b74 wordp=0x538f
// ================================================
// 0x538f: dw 0x0000 0x103d

// ================================================
// 0x5393: WORD '*2BUY' codep=0x3b74 wordp=0x539d
// ================================================
// 0x539d: dw 0x0000 0x1050

// ================================================
// 0x53a1: WORD 'MPS' codep=0x2214 wordp=0x53a9
// ================================================
// 0x53a9: dw 0x0001

// ================================================
// 0x53ab: WORD 'BOX-IAD' codep=0x3b74 wordp=0x53b7
// ================================================
// 0x53b7: dw 0x0000 0x1076

// ================================================
// 0x53bb: WORD '*DICT' codep=0x3b74 wordp=0x53c5
// ================================================
// 0x53c5: dw 0x0000 0x9760

// ================================================
// 0x53c9: WORD 'CLIP-TA' codep=0x1d29 wordp=0x53d5
// ================================================
// 0x53d5: db 0x08 0x08 0x01 0xc7 0x00 0x02 0x02 0x00 0x9f 0x00 0x04 0x04 0x01 0x00 0x00 0x01 0x01 0x00 0x00 0x00 '                    '

// ================================================
// 0x53e9: WORD 'TABLE' codep=0x224c wordp=0x53f3
// ================================================

void TABLE() // TABLE
{
  Exec("CREATE"); // call of word 0x1cbb '(CREATE)'
  CODE(); // (;CODE) inlined assembler code
// 0x53f7: call   1649
  SWAP(); // SWAP
  Push(Pop() * 2); //  2*
  Push(Pop() + Pop()); // +
  Push(Read16(Pop())); //  @
}


// ================================================
// 0x5404: WORD 'FONTSEG' codep=0x1d29 wordp=0x5410
// ================================================
// 0x5410: db 0x37 0x7f '7'

// ================================================
// 0x5412: WORD 'REMSEG' codep=0x1d29 wordp=0x541d
// ================================================
// 0x541d: db 0x3e 0x95 '> '

// ================================================
// 0x541f: WORD 'XBUF-SE' codep=0x1d29 wordp=0x542b
// ================================================
// 0x542b: db 0x37 0x6f '7o'

// ================================================
// 0x542d: WORD 'DICT' codep=0x1d29 wordp=0x5436
// ================================================
// 0x5436: db 0xfa 0x84 '  '

// ================================================
// 0x5438: WORD '?ALL' codep=0x1d29 wordp=0x5441
// ================================================
// 0x5441: db 0xff 0x00 '  '

// ================================================
// 0x5443: WORD '?YOK' codep=0x1d29 wordp=0x544c
// ================================================
// 0x544c: db 0xff 0x00 '  '

// ================================================
// 0x544e: WORD 'HUB' codep=0x1d29 wordp=0x5456
// ================================================
// 0x5456: db 0x01 0x00 '  '

// ================================================
// 0x5458: WORD 'PIC#' codep=0x1d29 wordp=0x5461
// ================================================
// 0x5461: db 0x5d 0x5e ']^'

// ================================================
// 0x5463: WORD '1LOGO' codep=0x1d29 wordp=0x546d
// ================================================
// 0x546d: db 0x1f 0x3f 0xdf 0xee 0x0c 0x33 0x0c 0x03 0xb7 0xcf 0x8f 0x7f 0x00 0xc0 ' ?   3       '

// ================================================
// 0x547b: WORD 'W547D' codep=0x1d29 wordp=0x547d
// ================================================
// 0x547d: db 0xc3 0x7e 0xa5 0xbd 0xa5 0xa5 0xc3 0xbd 0x00 0x7e ' ~       ~'

// ================================================
// 0x5487: WORD 'W5489' codep=0x1d29 wordp=0x5489
// ================================================
// 0x5489: db 0x18 0x7a 0x86 0x61 0x80 0x17 ' z a  '

// ================================================
// 0x548f: WORD 'W5491' codep=0x1d29 wordp=0x5491
// ================================================
// 0x5491: db 0x00 0xfc '  '

// ================================================
// 0x5493: WORD ''XCOMM' codep=0x1d29 wordp=0x549e
// ================================================
// 0x549e: db 0x32 0xc3 '2 '

// ================================================
// 0x54a0: WORD 'TCLR' codep=0x1d29 wordp=0x54a9
// ================================================
// 0x54a9: db 0x0f 0x00 '  '

// ================================================
// 0x54ab: WORD ''TRAK' codep=0x1d29 wordp=0x54b5
// ================================================
// 0x54b5: db 0x56 0xca 'V '

// ================================================
// 0x54b7: WORD 'TRAK-HR' codep=0x1d29 wordp=0x54c3
// ================================================
// 0x54c3: db 0x4e 0x20 'N '

// ================================================
// 0x54c5: WORD 'A-STREN' codep=0x1d29 wordp=0x54d1
// ================================================
// 0x54d1: db 0x48 0x20 'H '

// ================================================
// 0x54d3: WORD 'DIRBLK' codep=0x1d29 wordp=0x54de
// ================================================
// 0x54de: db 0x00 0x00 '  '

// ================================================
// 0x54e0: WORD 'TIMESTA' codep=0x1d29 wordp=0x54ec
// ================================================
// 0x54ec: db 0x20 0x01 '  '

// ================================================
// 0x54ee: WORD 'RELAXTI' codep=0x1d29 wordp=0x54fa
// ================================================

// ================================================
// 0x54fa: WORD 'W54FC' codep=0x7420 wordp=0x54fc
// ================================================
// orphan
IFieldType W54FC = {IDX, 0x54, 0x87};

// ================================================
// 0x54fc: WORD '):-,601' codep=0x1d29 wordp=0x5508
// ================================================
// 0x5508: db 0x0a 0x05 0x01 0x00 '    '

// ================================================
// 0x550c: WORD 'ICONFON' codep=0x1d29 wordp=0x5518
// ================================================
// 0x5518: db 0x01 0x00 '  '

// ================================================
// 0x551a: WORD 'LFRAME' codep=0x1d29 wordp=0x5525
// ================================================
// 0x5525: db 0x53 0x51 'SQ'

// ================================================
// 0x5527: WORD '?AUTO' codep=0x1d29 wordp=0x5531
// ================================================
// 0x5531: db 0x5d 0xdf '] '

// ================================================
// 0x5533: WORD '?CRITIC' codep=0x1d29 wordp=0x553f
// ================================================
// 0x553f: db 0x20 0x81 '  '

// ================================================
// 0x5541: WORD 'P-POSTU' codep=0x1d29 wordp=0x554d
// ================================================
// 0x554d: db 0x20 0x1e '  '

// ================================================
// 0x554f: WORD 'ELEM-AM' codep=0x1d29 wordp=0x555b
// ================================================
// 0x555b: db 0x14 0x00 '  '

// ================================================
// 0x555d: WORD ''INJURE' codep=0x1d29 wordp=0x5569
// ================================================
// 0x5569: db 0x68 0xce 'h '

// ================================================
// 0x556b: WORD 'P-COLOR' codep=0x1d29 wordp=0x5577
// ================================================
// 0x5577: db 0x41 0x4e 0x05 'AN '

// ================================================
// 0x557a: WORD 'P-PHRAS' codep=0x1d29 wordp=0x5586
// ================================================
// 0x5586: db 0x31 0x35 0x05 '15 '

// ================================================
// 0x5589: WORD 'P-CARP' codep=0x1d29 wordp=0x5594
// ================================================
// 0x5594: db 0x2e 0x8a 0x05 '.  '

// ================================================
// 0x5597: WORD 'O-COLOR' codep=0x1d29 wordp=0x55a3
// ================================================
// 0x55a3: db 0xeb 0x0e 0x05 '   '

// ================================================
// 0x55a6: WORD 'HYDRO' codep=0x1d29 wordp=0x55b0
// ================================================
// 0x55b0: db 0x5d 0x40 ']@'

// ================================================
// 0x55b2: WORD 'ATMO' codep=0x1d29 wordp=0x55bb
// ================================================
// 0x55bb: db 0x5d 0x00 '] '

// ================================================
// 0x55bd: WORD 'LCOLOR' codep=0x1d29 wordp=0x55c8
// ================================================
// 0x55c8: db 0x8e 0xde '  '

// ================================================
// 0x55ca: WORD '?TV' codep=0x1d29 wordp=0x55d2
// ================================================
// 0x55d2: db 0x01 0x00 '  '

// ================================================
// 0x55d4: WORD 'OLDHR' codep=0x1d29 wordp=0x55de
// ================================================
// 0x55de: db 0x36 0x75 '6u'

// ================================================
// 0x55e0: WORD 'AVCNT' codep=0x1d29 wordp=0x55ea
// ================================================
// 0x55ea: db 0x5d 0x2e '].'

// ================================================
// 0x55ec: WORD 'W55EE' codep=0x1d29 wordp=0x55ee
// ================================================
// 0x55ee: db 0x01 0x00 '  '

// ================================================
// 0x55f0: WORD '?BOMB' codep=0x1d29 wordp=0x55fa
// ================================================
// 0x55fa: db 0x00 0x00 0x00 '   '

// ================================================
// 0x55fd: WORD 'W55FF' codep=0x1d29 wordp=0x55ff
// ================================================
// 0x55ff: db 0x00 0x00 '  '

// ================================================
// 0x5601: WORD 'W5603' codep=0x1d29 wordp=0x5603
// ================================================
// 0x5603: db 0x00 0x00 '  '

// ================================================
// 0x5605: WORD 'W5607' codep=0x1d29 wordp=0x5607
// ================================================
// 0x5607: db 0xef 0xf9 '  '

// ================================================
// 0x5609: WORD 'FILE#' codep=0x1d29 wordp=0x5613
// ================================================
// 0x5613: db 0x12 0x00 '  '

// ================================================
// 0x5615: WORD 'RECORD#' codep=0x1d29 wordp=0x5621
// ================================================
// 0x5621: db 0x00 0x00 '  '

// ================================================
// 0x5623: WORD 'W5625' codep=0x1d29 wordp=0x5625
// ================================================
// 0x5625: db 0xd0 0xf7 '  '

// ================================================
// 0x5627: WORD 'W5629' codep=0x1d29 wordp=0x5629
// ================================================
// 0x5629: db 0xe0 0xfb '  '

// ================================================
// 0x562b: WORD 'CXSP' codep=0x1d29 wordp=0x5634
// ================================================
// 0x5634: db 0x7e 0x65 '~e'

// ================================================
// 0x5636: WORD 'W5638' codep=0x1d29 wordp=0x5638
// ================================================
// 0x5638: db 0xd1 0x65 ' e'

// ================================================
// 0x563a: WORD 'W563C' codep=0x1d29 wordp=0x563c
// ================================================
// orphan
// 0x563c: db 0x3a 0x20 ': '

// ================================================
// 0x563e: WORD 'EDL' codep=0x1d29 wordp=0x5646
// ================================================
// 0x5646: db 0x00 0x74 ' t'

// ================================================
// 0x5648: WORD 'A-POSTU' codep=0x1d29 wordp=0x5654
// ================================================
// 0x5654: db 0x20 0xa4 '  '

// ================================================
// 0x5656: WORD 'W5658' codep=0x1d29 wordp=0x5658
// ================================================
// 0x5658: db 0xc8 0x00 '  '

// ================================================
// 0x565a: WORD 'P-RACES' codep=0x1d29 wordp=0x5666
// ================================================
// 0x5666: db 0x20 0x2d ' -'

// ================================================
// 0x5668: WORD 'W566A' codep=0x1d29 wordp=0x566a
// ================================================
// 0x566a: db 0x8c 0x93 '  '

// ================================================
// 0x566c: WORD 'W566E' codep=0x1d29 wordp=0x566e
// ================================================
// 0x566e: db 0x6d 0x93 'm '

// ================================================
// 0x5670: WORD 'W5672' codep=0x1d29 wordp=0x5672
// ================================================
// 0x5672: db 0xca 0x93 '  '

// ================================================
// 0x5674: WORD 'W5676' codep=0x1d29 wordp=0x5676
// ================================================
// 0x5676: db 0x08 0x94 '  '

// ================================================
// 0x5678: WORD 'W567A' codep=0x1d29 wordp=0x567a
// ================================================
// orphan
// 0x567a: db 0x3a 0x20 ': '

// ================================================
// 0x567c: WORD ''THROW-' codep=0x1d29 wordp=0x5688
// ================================================
// 0x5688: db 0x6e 0x3a 'n:'

// ================================================
// 0x568a: WORD ''MAP' codep=0x1d29 wordp=0x5693
// ================================================
// 0x5693: db 0x48 0x3a 'H:'

// ================================================
// 0x5695: WORD ''TRAVER' codep=0x1d29 wordp=0x56a1
// ================================================
// 0x56a1: db 0xb3 0x7a ' z'

// ================================================
// 0x56a3: WORD ''?EXIT' codep=0x1d29 wordp=0x56ae
// ================================================
// 0x56ae: db 0xec 0x7d ' }'

// ================================================
// 0x56b0: WORD 'W56B2' codep=0x1d29 wordp=0x56b2
// ================================================
// 0x56b2: db 0x3a 0x20 ': '

// ================================================
// 0x56b4: WORD ''.FLUX-' codep=0x1d29 wordp=0x56c0
// ================================================
// 0x56c0: db 0xf6 0xe3 '  '

// ================================================
// 0x56c2: WORD '?TD' codep=0x1d29 wordp=0x56ca
// ================================================
// 0x56ca: db 0x00 0x00 '  '

// ================================================
// 0x56cc: WORD '?LANDED' codep=0x1d29 wordp=0x56d8
// ================================================
// 0x56d8: db 0x00 0x00 '  '

// ================================================
// 0x56da: WORD 'W56DC' codep=0x1d29 wordp=0x56dc
// ================================================
// 0x56dc: db 0xf0 0x01 '  '

// ================================================
// 0x56de: WORD 'W56E0' codep=0x1d29 wordp=0x56e0
// ================================================
// 0x56e0: db 0x00 0x00 '  '

// ================================================
// 0x56e2: WORD 'W56E4' codep=0x1d29 wordp=0x56e4
// ================================================
// 0x56e4: db 0x60 0x13 '` '

// ================================================
// 0x56e6: WORD 'W56E8' codep=0x1d29 wordp=0x56e8
// ================================================
// 0x56e8: db 0x00 0x00 '  '

// ================================================
// 0x56ea: WORD 'W56EC' codep=0x1d29 wordp=0x56ec
// ================================================
// 0x56ec: db 0x5a 0x0f 'Z '

// ================================================
// 0x56ee: WORD 'W56F0' codep=0x1d29 wordp=0x56f0
// ================================================
// 0x56f0: db 0x4d 0x12 'M '

// ================================================
// 0x56f2: WORD 'W56F4' codep=0x1d29 wordp=0x56f4
// ================================================
// 0x56f4: db 0x00 0x00 '  '

// ================================================
// 0x56f6: WORD 'W56F8' codep=0x1d29 wordp=0x56f8
// ================================================
// 0x56f8: db 0x0e 0x00 '  '

// ================================================
// 0x56fa: WORD '-END' codep=0x1d29 wordp=0x5703
// ================================================
// 0x5703: db 0x04 0x00 '  '

// ================================================
// 0x5705: WORD 'OV#' codep=0x1d29 wordp=0x570d
// ================================================
// 0x570d: db 0x00 0x00 '  '

// ================================================
// 0x570f: WORD 'W5711' codep=0x1d29 wordp=0x5711
// ================================================
// 0x5711: db 0x85 0x6f ' o'

// ================================================
// 0x5713: WORD 'REPAIRT' codep=0x1d29 wordp=0x571f
// ================================================
// 0x571f: db 0xff 0xff '  '

// ================================================
// 0x5721: WORD 'HBUF-SE' codep=0x1d29 wordp=0x572d
// ================================================
// 0x572d: db 0x37 0x6f '7o'

// ================================================
// 0x572f: WORD 'DBUF-SE' codep=0x1d29 wordp=0x573b
// ================================================
// 0x573b: db 0x00 0xa0 '  '

// ================================================
// 0x573d: WORD 'COLOR' codep=0x1d29 wordp=0x5747
// ================================================
// 0x5747: db 0x0f 0x00 '  '

// ================================================
// 0x5749: WORD 'DCOLOR' codep=0x1d29 wordp=0x5754
// ================================================
// 0x5754: db 0x0f 0x00 '  '

// ================================================
// 0x5756: WORD 'YTAB' codep=0x1d29 wordp=0x575f
// ================================================
// 0x575f: db 0x29 0x4c ')L'

// ================================================
// 0x5761: WORD 'Y1' codep=0x1d29 wordp=0x5768
// ================================================
// 0x5768: db 0x5e 0x89 '^ '

// ================================================
// 0x576a: WORD 'X1' codep=0x1d29 wordp=0x5771
// ================================================
// 0x5771: db 0x89 0x3e ' >'

// ================================================
// 0x5773: WORD 'Y2' codep=0x1d29 wordp=0x577a
// ================================================
// 0x577a: db 0x5e 0x5e '^^'

// ================================================
// 0x577c: WORD 'X2' codep=0x1d29 wordp=0x5783
// ================================================
// 0x5783: db 0xb2 0x08 '  '

// ================================================
// 0x5785: WORD 'YTABL' codep=0x1d29 wordp=0x578f
// ================================================
// 0x578f: db 0xe8 0xc7 '  '

// ================================================
// 0x5791: WORD 'BUF-SEG' codep=0x1d29 wordp=0x579d
// ================================================
// 0x579d: db 0x00 0xa0 '  '

// ================================================
// 0x579f: WORD 'RETURN' codep=0x1d29 wordp=0x57aa
// ================================================
// 0x57aa: db 0x1e 0xb8 '  '

// ================================================
// 0x57ac: WORD '?WIN' codep=0x1d29 wordp=0x57b5
// ================================================
// 0x57b5: db 0x00 0x00 '  '

// ================================================
// 0x57b7: WORD '#IN' codep=0x1d29 wordp=0x57bf
// ================================================
// 0x57bf: db 0x04 0x00 '  '

// ================================================
// 0x57c1: WORD '#OUT' codep=0x1d29 wordp=0x57ca
// ================================================
// 0x57ca: db 0x41 0x42 'AB'

// ================================================
// 0x57cc: WORD 'VIN' codep=0x1d29 wordp=0x57d4
// ================================================
// 0x57d4: db 0x8b 0x68 ' h'

// ================================================
// 0x57d6: WORD 'VOUT' codep=0x1d29 wordp=0x57df
// ================================================
// 0x57df: db 0xcd 0x68 ' h'

// ================================================
// 0x57e1: WORD 'OIN' codep=0x1d29 wordp=0x57e9
// ================================================
// 0x57e9: db 0x0f 0x69 ' i'

// ================================================
// 0x57eb: WORD 'OOUT' codep=0x1d29 wordp=0x57f4
// ================================================
// 0x57f4: db 0x51 0x69 'Qi'

// ================================================
// 0x57f6: WORD 'IVPTR' codep=0x1d29 wordp=0x5800
// ================================================
// 0x5800: db 0x0d 0x3d ' ='

// ================================================
// 0x5802: WORD 'OVPTR' codep=0x1d29 wordp=0x580c
// ================================================
// 0x580c: db 0xeb 0x06 '  '

// ================================================
// 0x580e: WORD 'ICPTR' codep=0x1d29 wordp=0x5818
// ================================================
// 0x5818: db 0x16 0x80 '  '

// ================================================
// 0x581a: WORD 'OCPTR' codep=0x1d29 wordp=0x5824
// ================================================
// 0x5824: db 0xb8 0xff '  '

// ================================================
// 0x5826: WORD 'FLIP' codep=0x1d29 wordp=0x582f
// ================================================
// 0x582f: db 0x83 0xfb '  '

// ================================================
// 0x5831: WORD 'TACCPT' codep=0x1d29 wordp=0x583c
// ================================================
// 0x583c: db 0x06 0x0b '  '

// ================================================
// 0x583e: WORD 'TRJCT' codep=0x1d29 wordp=0x5848
// ================================================
// 0x5848: db 0x29 0x52 ')R'

// ================================================
// 0x584a: WORD '?OPEN' codep=0x1d29 wordp=0x5854
// ================================================
// 0x5854: db 0x00 0x00 '  '

// ================================================
// 0x5856: WORD '?EVAL' codep=0x1d29 wordp=0x5860
// ================================================
// 0x5860: db 0x00 0x00 '  '

// ================================================
// 0x5862: WORD 'SX' codep=0x1d29 wordp=0x5869
// ================================================
// 0x5869: db 0x3c 0x00 '< '

// ================================================
// 0x586b: WORD 'SY' codep=0x1d29 wordp=0x5872
// ================================================
// 0x5872: db 0xc2 0x00 '  '

// ================================================
// 0x5874: WORD 'SO' codep=0x1d29 wordp=0x587b
// ================================================
// 0x587b: db 0x20 0x53 ' S'

// ================================================
// 0x587d: WORD 'PX' codep=0x1d29 wordp=0x5884
// ================================================
// 0x5884: db 0x3c 0x00 '< '

// ================================================
// 0x5886: WORD 'PY' codep=0x1d29 wordp=0x588d
// ================================================
// 0x588d: db 0xc6 0x00 '  '

// ================================================
// 0x588f: WORD 'PO' codep=0x1d29 wordp=0x5896
// ================================================
// 0x5896: db 0x72 0x66 'rf'

// ================================================
// 0x5898: WORD 'IX' codep=0x1d29 wordp=0x589f
// ================================================
// 0x589f: db 0x39 0x29 '9)'

// ================================================
// 0x58a1: WORD 'IY' codep=0x1d29 wordp=0x58a8
// ================================================
// 0x58a8: db 0x1e 0x70 ' p'

// ================================================
// 0x58aa: WORD 'IO' codep=0x1d29 wordp=0x58b1
// ================================================
// 0x58b1: db 0xff 0x74 ' t'

// ================================================
// 0x58b3: WORD 'COMPARE' codep=0x1d29 wordp=0x58bf
// ================================================
// 0x58bf: db 0x4f 0x44 'OD'

// ================================================
// 0x58c1: WORD 'X-INTER' codep=0x1d29 wordp=0x58cd
// ================================================
// 0x58cd: db 0x19 0x80 '  '

// ================================================
// 0x58cf: WORD 'ILEFT' codep=0x1d29 wordp=0x58d9
// ================================================
// 0x58d9: db 0x00 0x00 '  '

// ================================================
// 0x58db: WORD 'IRIGHT' codep=0x1d29 wordp=0x58e6
// ================================================
// 0x58e6: db 0x9f 0x00 '  '

// ================================================
// 0x58e8: WORD 'IBELOW' codep=0x1d29 wordp=0x58f3
// ================================================
// 0x58f3: db 0x00 0x00 '  '

// ================================================
// 0x58f5: WORD 'IABOVE' codep=0x1d29 wordp=0x5900
// ================================================
// 0x5900: db 0xc7 0x00 '  '

// ================================================
// 0x5902: WORD ''FLY' codep=0x1d29 wordp=0x590b
// ================================================
// 0x590b: db 0x92 0xca '  '

// ================================================
// 0x590d: WORD ''UNNEST' codep=0x1d29 wordp=0x5919
// ================================================
// 0x5919: db 0xfc 0xca '  '

// ================================================
// 0x591b: WORD '?NEW' codep=0x1d29 wordp=0x5924
// ================================================
// 0x5924: db 0x00 0x00 '  '

// ================================================
// 0x5926: WORD 'FORCED' codep=0x1d29 wordp=0x5931
// ================================================
// 0x5931: db 0x21 0x5a '!Z'

// ================================================
// 0x5933: WORD '#VESS' codep=0x1d29 wordp=0x593d
// ================================================
// 0x593d: db 0x00 0x00 '  '

// ================================================
// 0x593f: WORD 'CTCOLOR' codep=0x1d29 wordp=0x594b
// ================================================
// 0x594b: db 0x5a 0x5b 'Z['

// ================================================
// 0x594d: WORD 'XSTART' codep=0x1d29 wordp=0x5958
// ================================================
// 0x5958: db 0x60 0x00 '` '

// ================================================
// 0x595a: WORD 'XEND' codep=0x1d29 wordp=0x5963
// ================================================
// 0x5963: db 0x79 0x00 'y '

// ================================================
// 0x5965: WORD 'YLINE' codep=0x1d29 wordp=0x596f
// ================================================
// 0x596f: db 0xc6 0x00 '  '

// ================================================
// 0x5971: WORD '?3' codep=0x1d29 wordp=0x5978
// ================================================
// 0x5978: db 0x00 0x00 '  '

// ================================================
// 0x597a: WORD ''.HUFF' codep=0x1d29 wordp=0x5985
// ================================================
// 0x5985: db 0x45 0xbe 'E '

// ================================================
// 0x5987: WORD 'SCAN' codep=0x1d29 wordp=0x5990
// ================================================
// 0x5990: db 0xf9 0x66 ' f'

// ================================================
// 0x5992: WORD 'LMNT' codep=0x1d29 wordp=0x599b
// ================================================
// 0x599b: db 0x00 0x00 '  '

// ================================================
// 0x599d: WORD 'SCAN+' codep=0x1d29 wordp=0x59a7
// ================================================
// 0x59a7: db 0xf9 0x66 ' f'

// ================================================
// 0x59a9: WORD 'YMIN' codep=0x1d29 wordp=0x59b2
// ================================================
// 0x59b2: db 0xc2 0x00 '  '

// ================================================
// 0x59b4: WORD 'YMAX' codep=0x1d29 wordp=0x59bd
// ================================================
// 0x59bd: db 0xc6 0x00 '  '

// ================================================
// 0x59bf: WORD '#HORIZ' codep=0x1d29 wordp=0x59ca
// ================================================
// 0x59ca: db 0x02 0x00 '  '

// ================================================
// 0x59cc: WORD 'MOVED' codep=0x1d29 wordp=0x59d6
// ================================================
// 0x59d6: db 0x69 0x7a 'iz'

// ================================================
// 0x59d8: WORD 'MEMSEG' codep=0x1d29 wordp=0x59e3
// ================================================
// 0x59e3: db 0x19 0x42 ' B'

// ================================================
// 0x59e5: WORD 'MEMOFF' codep=0x1d29 wordp=0x59f0
// ================================================
// 0x59f0: db 0x00 0x0a '  '

// ================================================
// 0x59f2: WORD 'MONITOR' codep=0x1d29 wordp=0x59fe
// ================================================
// 0x59fe: db 0x06 0x00 '  '

// ================================================
// 0x5a00: WORD 'LOCRADI' codep=0x1d29 wordp=0x5a0c
// ================================================
// 0x5a0c: db 0x4b 0x00 'K '

// ================================================
// 0x5a0e: WORD '(ELIST)' codep=0x1d29 wordp=0x5a1a
// ================================================
// 0x5a1a: db 0xe8 0x10 0x00 '   '

// ================================================
// 0x5a1d: WORD 'ICON^' codep=0x1d29 wordp=0x5a27
// ================================================
// 0x5a27: db 0x78 0x69 'xi'

// ================================================
// 0x5a29: WORD 'PAL^' codep=0x1d29 wordp=0x5a32
// ================================================
// 0x5a32: db 0x41 0x6a 'Aj'

// ================================================
// 0x5a34: WORD 'YBLT' codep=0x1d29 wordp=0x5a3d
// ================================================
// 0x5a3d: db 0xc6 0x00 '  '

// ================================================
// 0x5a3f: WORD 'XBLT' codep=0x1d29 wordp=0x5a48
// ================================================
// 0x5a48: db 0x40 0x00 '@ '

// ================================================
// 0x5a4a: WORD 'XORMODE' codep=0x1d29 wordp=0x5a56
// ================================================
// 0x5a56: db 0x00 0x00 '  '

// ================================================
// 0x5a58: WORD 'LBLT' codep=0x1d29 wordp=0x5a61
// ================================================
// 0x5a61: db 0x08 0x00 '  '

// ================================================
// 0x5a63: WORD 'WBLT' codep=0x1d29 wordp=0x5a6c
// ================================================
// 0x5a6c: db 0x08 0x00 '  '

// ================================================
// 0x5a6e: WORD 'ABLT' codep=0x1d29 wordp=0x5a77
// ================================================
// 0x5a77: db 0x40 0x02 '@ '

// ================================================
// 0x5a79: WORD 'BLTSEG' codep=0x1d29 wordp=0x5a84
// ================================================
// 0x5a84: db 0xf2 0x9d '  '

// ================================================
// 0x5a86: WORD 'BLT>' codep=0x1d29 wordp=0x5a8f
// ================================================
// 0x5a8f: db 0x42 0x02 'B '

// ================================================
// 0x5a91: WORD 'TILE-PT' codep=0x1d29 wordp=0x5a9d
// ================================================
// 0x5a9d: db 0x9a 0x69 ' i'

// ================================================
// 0x5a9f: WORD '?FUEL-D' codep=0x1d29 wordp=0x5aab
// ================================================
// 0x5aab: db 0x00 0x00 '  '

// ================================================
// 0x5aad: WORD '?G-AWAR' codep=0x1d29 wordp=0x5ab9
// ================================================
// 0x5ab9: db 0x00 0x00 '  '

// ================================================
// 0x5abb: WORD 'GWF' codep=0x1d29 wordp=0x5ac3
// ================================================
// 0x5ac3: db 0x00 0x00 '  '

// ================================================
// 0x5ac5: WORD 'DXVIS' codep=0x1d29 wordp=0x5acf
// ================================================
// 0x5acf: db 0x41 0x53 'AS'

// ================================================
// 0x5ad1: WORD 'DYVIS' codep=0x1d29 wordp=0x5adb
// ================================================
// 0x5adb: db 0x45 0x20 'E '

// ================================================
// 0x5add: WORD 'XCON' codep=0x1d29 wordp=0x5ae6
// ================================================
// 0x5ae6: db 0x56 0x20 'V '

// ================================================
// 0x5ae8: WORD 'YCON' codep=0x1d29 wordp=0x5af1
// ================================================
// 0x5af1: db 0x65 0x72 'er'

// ================================================
// 0x5af3: WORD 'DXCON' codep=0x1d29 wordp=0x5afd
// ================================================
// 0x5afd: db 0x72 0x20 'r '

// ================================================
// 0x5aff: WORD 'DYCON' codep=0x1d29 wordp=0x5b09
// ================================================
// 0x5b09: db 0x6e 0x20 'n '

// ================================================
// 0x5b0b: WORD 'XVIS' codep=0x1d29 wordp=0x5b14
// ================================================
// 0x5b14: db 0x69 0x6e 'in'

// ================================================
// 0x5b16: WORD 'YVIS' codep=0x1d29 wordp=0x5b1f
// ================================================
// 0x5b1f: db 0x65 0x72 'er'

// ================================================
// 0x5b21: WORD 'XLLDEST' codep=0x1d29 wordp=0x5b2d
// ================================================
// 0x5b2d: db 0x00 0x00 '  '

// ================================================
// 0x5b2f: WORD 'YLLDEST' codep=0x1d29 wordp=0x5b3b
// ================================================
// 0x5b3b: db 0x00 0x00 '  '

// ================================================
// 0x5b3d: WORD 'GLOBALS' codep=0x1d29 wordp=0x5b49
// ================================================
// 0x5b49: db 0x44 0x20 'D '

// ================================================
// 0x5b4b: WORD ''.CELL' codep=0x1d29 wordp=0x5b56
// ================================================
// 0x5b56: db 0x72 0x72 'rr'

// ================================================
// 0x5b58: WORD ''.BACKG' codep=0x1d29 wordp=0x5b64
// ================================================
// 0x5b64: db 0x55 0x4e 'UN'

// ================================================
// 0x5b66: WORD ''ICON-P' codep=0x1d29 wordp=0x5b72
// ================================================
// 0x5b72: db 0x55 0xba 'U '

// ================================================
// 0x5b74: WORD ''ICONBO' codep=0x1d29 wordp=0x5b80
// ================================================
// 0x5b80: db 0x65 0xba 'e '

// ================================================
// 0x5b82: WORD ''CC' codep=0x1d29 wordp=0x5b8a
// ================================================
// 0x5b8a: db 0x3c 0xc8 '< '

// ================================================
// 0x5b8c: WORD 'W5B8E' codep=0x1d29 wordp=0x5b8e
// ================================================
// 0x5b8e: db 0x13 0x93 '  '

// ================================================
// 0x5b90: WORD 'W5B92' codep=0x1d29 wordp=0x5b92
// ================================================
// 0x5b92: db 0xb9 0x92 '  '

// ================================================
// 0x5b94: WORD 'W5B96' codep=0x1d29 wordp=0x5b96
// ================================================
// 0x5b96: db 0x8c 0x92 '  '

// ================================================
// 0x5b98: WORD 'W5B9A' codep=0x1d29 wordp=0x5b9a
// ================================================
// 0x5b9a: db 0x5f 0x92 '_ '

// ================================================
// 0x5b9c: WORD 'W5B9E' codep=0x1d29 wordp=0x5b9e
// ================================================
// 0x5b9e: db 0x05 0x92 '  '

// ================================================
// 0x5ba0: WORD 'IHSEG' codep=0x1d29 wordp=0x5baa
// ================================================
// 0x5baa: db 0xd8 0x91 '  '

// ================================================
// 0x5bac: WORD 'IGLOBAL' codep=0x1d29 wordp=0x5bb8
// ================================================
// 0x5bb8: db 0x00 0x00 '  '

// ================================================
// 0x5bba: WORD 'ILOCAL' codep=0x1d29 wordp=0x5bc5
// ================================================
// 0x5bc5: db 0x00 0x00 '  '

// ================================================
// 0x5bc7: WORD 'IINDEX' codep=0x1d29 wordp=0x5bd2
// ================================================
// 0x5bd2: db 0x00 0x00 '  '

// ================================================
// 0x5bd4: WORD 'XWLL' codep=0x1d29 wordp=0x5bdd
// ================================================
// 0x5bdd: db 0x64 0x65 'de'

// ================================================
// 0x5bdf: WORD 'YWLL' codep=0x1d29 wordp=0x5be8
// ================================================

// ================================================
// 0x5be8: WORD 'W5BEA' codep=0x7420 wordp=0x5bea
// ================================================
// orphan
IFieldType W5BEA = {IDX, 0x5b, 0x84};

// ================================================
// 0x5bea: WORD 'XWUR' codep=0x1d29 wordp=0x5bf3
// ================================================
// 0x5bf3: db 0x72 0x64 'rd'

// ================================================
// 0x5bf5: WORD 'YWUR' codep=0x1d29 wordp=0x5bfe
// ================================================
// 0x5bfe: db 0x20 0x77 ' w'

// ================================================
// 0x5c00: WORD '*GLOBAL' codep=0x1d29 wordp=0x5c0c
// ================================================
// 0x5c0c: db 0xa9 0x01 '  '

// ================================================
// 0x5c0e: WORD '(STOP-C' codep=0x1d29 wordp=0x5c1a
// ================================================
// 0x5c1a: db 0x4d 0x29 'M)'

// ================================================
// 0x5c1c: WORD 'W5C1E' codep=0x1d29 wordp=0x5c1e
// ================================================
// 0x5c1e: db 0x00 0x00 '  '

// ================================================
// 0x5c20: WORD 'CONTEXT_3' codep=0x1d29 wordp=0x5c2c
// ================================================
// 0x5c2c: db 0x03 0x00 '  '

// ================================================
// 0x5c2e: WORD '%EFF' codep=0x1d29 wordp=0x5c37
// ================================================
// 0x5c37: db 0x64 0x00 'd '

// ================================================
// 0x5c39: WORD 'STORM' codep=0x1d29 wordp=0x5c43
// ================================================
// 0x5c43: db 0x00 0x00 '  '

// ================================================
// 0x5c45: WORD ''TVT' codep=0x1d29 wordp=0x5c4e
// ================================================
// 0x5c4e: db 0x89 0xd1 '  '

// ================================================
// 0x5c50: WORD ''STORM' codep=0x1d29 wordp=0x5c5b
// ================================================
// 0x5c5b: db 0x5a 0xce 'Z '

// ================================================
// 0x5c5d: WORD 'E/M' codep=0x1d29 wordp=0x5c65
// ================================================
// 0x5c65: db 0x66 0x6f 'fo'

// ================================================
// 0x5c67: WORD 'FORCEPT' codep=0x1d29 wordp=0x5c73
// ================================================
// 0x5c73: db 0x00 0x00 '  '

// ================================================
// 0x5c75: WORD '#STORM' codep=0x1d29 wordp=0x5c80
// ================================================
// 0x5c80: db 0x00 0x00 '  '

// ================================================
// 0x5c82: WORD 'W5C84' codep=0x1d29 wordp=0x5c84
// ================================================
// orphan
// 0x5c84: db 0x4a 0x00 'J '

// ================================================
// 0x5c86: WORD 'W5C88' codep=0x1d29 wordp=0x5c88
// ================================================
// orphan
// 0x5c88: db 0x00 0x00 '  '

// ================================================
// 0x5c8a: WORD 'W5C8C' codep=0x1d29 wordp=0x5c8c
// ================================================
// orphan
// 0x5c8c: db 0x4b 0x00 'K '

// ================================================
// 0x5c8e: WORD 'W5C90' codep=0x1d29 wordp=0x5c90
// ================================================
// 0x5c90: db 0x00 0x00 '  '

// ================================================
// 0x5c92: WORD 'PORTDAT' codep=0x1d29 wordp=0x5c9e
// ================================================
// 0x5c9e: db 0x00 0x00 '  '

// ================================================
// 0x5ca0: WORD '?PORT' codep=0x1d29 wordp=0x5caa
// ================================================
// 0x5caa: db 0x01 0x00 '  '

// ================================================
// 0x5cac: WORD 'TVIS' codep=0x1d29 wordp=0x5cb5
// ================================================
// 0x5cb5: db 0x40 0x00 '@ '

// ================================================
// 0x5cb7: WORD 'RVIS' codep=0x1d29 wordp=0x5cc0
// ================================================
// 0x5cc0: db 0x88 0x00 '  '

// ================================================
// 0x5cc2: WORD 'BVIS' codep=0x1d29 wordp=0x5ccb
// ================================================
// 0x5ccb: db 0xf7 0xff '  '

// ================================================
// 0x5ccd: WORD 'LVIS' codep=0x1d29 wordp=0x5cd6
// ================================================
// 0x5cd6: db 0x42 0x00 'B '

// ================================================
// 0x5cd8: WORD 'LFSEG' codep=0x1d29 wordp=0x5ce2
// ================================================
// 0x5ce2: db 0x3e 0x95 '> '

// ================================================
// 0x5ce4: WORD 'LSYSEG' codep=0x1d29 wordp=0x5cef
// ================================================
// 0x5cef: db 0x51 0x91 'Q '

// ================================================
// 0x5cf1: WORD 'MSYSEG' codep=0x1d29 wordp=0x5cfc
// ================================================
// 0x5cfc: db 0xd7 0x90 '  '

// ================================================
// 0x5cfe: WORD 'SSYSEG' codep=0x1d29 wordp=0x5d09
// ================================================
// 0x5d09: db 0x5d 0x90 '] '

// ================================================
// 0x5d0b: WORD '?REPAIR' codep=0x1d29 wordp=0x5d17
// ================================================
// 0x5d17: db 0x00 0x00 '  '

// ================================================
// 0x5d19: WORD '?HEAL' codep=0x1d29 wordp=0x5d23
// ================================================
// 0x5d23: db 0x00 0x00 '  '

// ================================================
// 0x5d25: WORD 'MXNEB' codep=0x1d29 wordp=0x5d2f
// ================================================
// 0x5d2f: db 0x41 0x53 'AS'

// ================================================
// 0x5d31: WORD 'THIS-BU' codep=0x1d29 wordp=0x5d3d
// ================================================
// 0x5d3d: db 0x00 0x00 '  '

// ================================================
// 0x5d3f: WORD 'NCRS' codep=0x1d29 wordp=0x5d48
// ================================================
// 0x5d48: db 0x20 0x62 ' b'

// ================================================
// 0x5d4a: WORD 'OCRS' codep=0x1d29 wordp=0x5d53
// ================================================
// 0x5d53: db 0x73 0x6f 'so'

// ================================================
// 0x5d55: WORD 'WTOP' codep=0x1d29 wordp=0x5d5e
// ================================================
// 0x5d5e: db 0x33 0x00 '3 '

// ================================================
// 0x5d60: WORD 'WBOTTOM' codep=0x1d29 wordp=0x5d6c
// ================================================
// 0x5d6c: db 0x01 0x00 '  '

// ================================================
// 0x5d6e: WORD 'WRIGHT' codep=0x1d29 wordp=0x5d79
// ================================================
// 0x5d79: db 0x9e 0x00 '  '

// ================================================
// 0x5d7b: WORD 'WLEFT' codep=0x1d29 wordp=0x5d85
// ================================================
// 0x5d85: db 0x03 0x00 '  '

// ================================================
// 0x5d87: WORD 'WLINES' codep=0x1d29 wordp=0x5d92
// ================================================
// 0x5d92: db 0x07 0x00 '  '

// ================================================
// 0x5d94: WORD 'WCHARS' codep=0x1d29 wordp=0x5d9f
// ================================================
// 0x5d9f: db 0x26 0x00 '& '

// ================================================
// 0x5da1: WORD 'SKIP2NE' codep=0x1d29 wordp=0x5dad
// ================================================
// 0x5dad: db 0x00 0x00 '  '

// ================================================
// 0x5daf: WORD '-AIN' codep=0x1d29 wordp=0x5db8
// ================================================
// 0x5db8: db 0x74 0x20 't '

// ================================================
// 0x5dba: WORD ''LAUNCH' codep=0x1d29 wordp=0x5dc6
// ================================================
// 0x5dc6: db 0x54 0xd4 'T '

// ================================================
// 0x5dc8: WORD '?ON-PLA' codep=0x1d29 wordp=0x5dd4
// ================================================
// 0x5dd4: db 0x00 0x00 '  '

// ================================================
// 0x5dd6: WORD '?RECALL' codep=0x1d29 wordp=0x5de2
// ================================================
// 0x5de2: db 0x00 0x00 '  '

// ================================================
// 0x5de4: WORD 'WMSG' codep=0x1d29 wordp=0x5ded
// ================================================
// 0x5ded: db 0x00 0x00 '  '

// ================================================
// 0x5def: WORD 'CTX' codep=0x1d29 wordp=0x5df7
// ================================================
// 0x5df7: db 0x00 0x00 '  '

// ================================================
// 0x5df9: WORD 'CTY' codep=0x1d29 wordp=0x5e01
// ================================================
// 0x5e01: db 0x00 0x00 '  '

// ================================================
// 0x5e03: WORD 'FTRIG' codep=0x1d29 wordp=0x5e0d
// ================================================
// 0x5e0d: db 0x00 0x00 '  '

// ================================================
// 0x5e0f: WORD 'FQUIT' codep=0x1d29 wordp=0x5e19
// ================================================
// 0x5e19: db 0x74 0x61 'ta'

// ================================================
// 0x5e1b: WORD 'LKEY' codep=0x1d29 wordp=0x5e24
// ================================================
// 0x5e24: db 0x73 0x74 'st'

// ================================================
// 0x5e26: WORD ''BUTTON' codep=0x1d29 wordp=0x5e32
// ================================================
// 0x5e32: db 0x65 0x20 'e '

// ================================================
// 0x5e34: WORD 'BTN-REC' codep=0x1d29 wordp=0x5e40
// ================================================
// 0x5e40: db 0x63 0x74 'ct'

// ================================================
// 0x5e42: WORD 'CRSCOLO' codep=0x1d29 wordp=0x5e4e
// ================================================
// 0x5e4e: db 0x6c 0x65 'le'

// ================================================
// 0x5e50: WORD 'W5E52' codep=0x1d29 wordp=0x5e52
// ================================================
// 0x5e52: db 0x3a 0x20 ': '

// ================================================
// 0x5e54: WORD '?>OP' codep=0x1d29 wordp=0x5e5d
// ================================================
// 0x5e5d: db 0x00 0x00 '  '

// ================================================
// 0x5e5f: WORD ''YANK' codep=0x1d29 wordp=0x5e69
// ================================================
// 0x5e69: db 0x3f 0xcb '? '

// ================================================
// 0x5e6b: WORD '?12' codep=0x1d29 wordp=0x5e73
// ================================================
// 0x5e73: db 0x76 0x61 'va'

// ================================================
// 0x5e75: WORD ''+VESS' codep=0x1d29 wordp=0x5e80
// ================================================
// 0x5e80: db 0x44 0xca 'D '

// ================================================
// 0x5e82: WORD '?NEB' codep=0x1d29 wordp=0x5e8b
// ================================================
// 0x5e8b: db 0x00 0x00 '  '

// ================================================
// 0x5e8d: WORD 'FORCEKE' codep=0x1d29 wordp=0x5e99
// ================================================
// 0x5e99: db 0x00 0x00 '  '

// ================================================
// 0x5e9b: WORD '%VAL' codep=0x1d29 wordp=0x5ea4
// ================================================
// 0x5ea4: db 0x64 0x00 'd '

// ================================================
// 0x5ea6: WORD 'SCROLL-' codep=0x1d29 wordp=0x5eb2
// ================================================
// 0x5eb2: db 0x4e 0x20 'N '

// ================================================
// 0x5eb4: WORD '[#CACHE' codep=0x1d29 wordp=0x5ec0
// ================================================
// 0x5ec0: db 0x99 0x00 '  '

// ================================================
// 0x5ec2: WORD 'ESC-EN' codep=0x1d29 wordp=0x5ecd
// ================================================
// 0x5ecd: db 0x00 0x00 '  '

// ================================================
// 0x5ecf: WORD 'ESC-PFA' codep=0x1d29 wordp=0x5edb
// ================================================
// 0x5edb: db 0x99 0xd5 '  '

// ================================================
// 0x5edd: WORD 'LINE-CO' codep=0x1d29 wordp=0x5ee9
// ================================================
// 0x5ee9: db 0x54 0x20 'T '

// ================================================
// 0x5eeb: WORD 'PM-PTR' codep=0x1d29 wordp=0x5ef6
// ================================================
// 0x5ef6: db 0x6f 0x75 'ou'

// ================================================
// 0x5ef8: WORD 'W5EFA' codep=0x1d29 wordp=0x5efa
// ================================================
// 0x5efa: db 0x01 0x00 '  '

// ================================================
// 0x5efc: WORD 'SKEY' codep=0x1d29 wordp=0x5f05
// ================================================
// 0x5f05: db 0x63 0x72 'cr'

// ================================================
// 0x5f07: WORD '#AUX' codep=0x1d29 wordp=0x5f10
// ================================================
// 0x5f10: db 0x05 0x00 '  '

// ================================================
// 0x5f12: WORD '?EGA' codep=0x1d29 wordp=0x5f1b
// ================================================
// 0x5f1b: db 0x00 0x00 '  '

// ================================================
// 0x5f1d: WORD '?5:(' codep=0x1d29 wordp=0x5f26
// ================================================
// 0x5f26: db 0x00 0x00 '  '

// ================================================
// 0x5f28: WORD 'XABS' codep=0x1d29 wordp=0x5f31
// ================================================
// 0x5f31: db 0x4a 0x00 'J '

// ================================================
// 0x5f33: WORD 'YABS' codep=0x1d29 wordp=0x5f3c
// ================================================
// 0x5f3c: db 0x00 0x00 '  '

// ================================================
// 0x5f3e: WORD 'HEADING' codep=0x1d29 wordp=0x5f4a
// ================================================
// 0x5f4a: db 0x04 0x00 '  '

// ================================================
// 0x5f4c: WORD '3DSEG' codep=0x1d29 wordp=0x5f56
// ================================================
// 0x5f56: db 0x69 0x70 'ip'

// ================================================
// 0x5f58: WORD 'VIN'' codep=0x1d29 wordp=0x5f61
// ================================================
// 0x5f61: db 0x67 0x20 'g '

// ================================================
// 0x5f63: WORD 'YSCREEN' codep=0x1d29 wordp=0x5f6f
// ================================================

// ================================================
// 0x5f6f: WORD 'W5F71' codep=0x7420 wordp=0x5f71
// ================================================
// orphan
IFieldType W5F71 = {ICON1_4IDX, 0x5f, 0x87};

// ================================================
// 0x5f71: WORD 'XSCREEN' codep=0x1d29 wordp=0x5f7d
// ================================================
// 0x5f7d: db 0x6f 0x72 'or'

// ================================================
// 0x5f7f: WORD ''COMBAT' codep=0x1d29 wordp=0x5f8b
// ================================================
// 0x5f8b: db 0x7b 0xc8 '{ '

// ================================================
// 0x5f8d: WORD ''CEX+' codep=0x1d29 wordp=0x5f97
// ================================================
// 0x5f97: db 0xa1 0xbf '  '

// ================================================
// 0x5f99: WORD ''CEX' codep=0x1d29 wordp=0x5fa2
// ================================================
// 0x5fa2: db 0xb5 0xbf '  '

// ================================================
// 0x5fa4: WORD ''WAX' codep=0x1d29 wordp=0x5fad
// ================================================
// 0x5fad: db 0x6b 0xc8 'k '

// ================================================
// 0x5faf: WORD 'TERMINA' codep=0x1d29 wordp=0x5fbb
// ================================================
// 0x5fbb: db 0x44 0x20 'D '

// ================================================
// 0x5fbd: WORD '?COMBAT' codep=0x1d29 wordp=0x5fc9
// ================================================
// 0x5fc9: db 0x00 0x00 '  '

// ================================================
// 0x5fcb: WORD '?ATTACK' codep=0x1d29 wordp=0x5fd7
// ================================================
// 0x5fd7: db 0x74 0x6f 'to'

// ================================================
// 0x5fd9: WORD 'TBOX' codep=0x1d29 wordp=0x5fe2
// ================================================
// 0x5fe2: db 0x65 0x61 'ea'

// ================================================
// 0x5fe4: WORD 'W5FE6' codep=0x1d29 wordp=0x5fe6
// ================================================
// 0x5fe6: db 0x00 0x00 '  '

// ================================================
// 0x5fe8: WORD 'STAR-HR' codep=0x1d29 wordp=0x5ff4
// ================================================
// 0x5ff4: db 0x00 0x00 '  '

// ================================================
// 0x5ff6: WORD 'STARDAT' codep=0x1d29 wordp=0x6002
// ================================================
// 0x6002: db 0x00 0x00 '  '

// ================================================
// 0x6004: WORD 'TIME-PA' codep=0x1d29 wordp=0x6010
// ================================================
// 0x6010: db 0x49 0x4e 'IN'

// ================================================
// 0x6012: WORD '#CLRMAP' codep=0x1d29 wordp=0x601e
// ================================================
// 0x601e: db 0x01 0x00 '  '

// ================================================
// 0x6020: WORD 'PLHI' codep=0x1d29 wordp=0x6029
// ================================================
// 0x6029: db 0xff 0xff '  '

// ================================================
// 0x602b: WORD ''PROCES' codep=0x1d29 wordp=0x6037
// ================================================
// 0x6037: db 0x75 0x72 'ur'

// ================================================
// 0x6039: WORD 'CURSEG' codep=0x1d29 wordp=0x6044
// ================================================
// 0x6044: db 0xcb 0x91 '  '

// ================================================
// 0x6046: WORD ''SIMULA' codep=0x1d29 wordp=0x6052
// ================================================
// 0x6052: db 0xb6 0xd1 '  '

// ================================================
// 0x6054: WORD ''ENDING' codep=0x1d29 wordp=0x6060
// ================================================
// 0x6060: db 0xe0 0xc8 '  '

// ================================================
// 0x6062: WORD '[KEYINT' codep=0x1d29 wordp=0x606e
// ================================================
// 0x606e: db 0x00 0x00 '  '

// ================================================
// 0x6070: WORD 'SIL^' codep=0x1d29 wordp=0x6079
// ================================================
// 0x6079: db 0x69 0x6a 'ij'

// ================================================
// 0x607b: WORD 'PIC^' codep=0x1d29 wordp=0x6084
// ================================================
// 0x6084: db 0x37 0x6b '7k'

// ================================================
// 0x6086: WORD ''CLEANU' codep=0x1d29 wordp=0x6092
// ================================================
// 0x6092: db 0x20 0x2d ' -'

// ================================================
// 0x6094: WORD ''KEY-CA' codep=0x1d29 wordp=0x60a0
// ================================================
// 0x60a0: db 0x20 0x6d ' m'

// ================================================
// 0x60a2: WORD ''.VITAL' codep=0x1d29 wordp=0x60ae
// ================================================
// 0x60ae: db 0x49 0x47 'IG'

// ================================================
// 0x60b0: WORD ''.DATE' codep=0x1d29 wordp=0x60bb
// ================================================
// 0x60bb: db 0x69 0x74 'it'

// ================================================
// 0x60bd: WORD ''.VEHIC' codep=0x1d29 wordp=0x60c9
// ================================================
// 0x60c9: db 0x2d 0x53 '-S'

// ================================================
// 0x60cb: WORD ''VEHICL' codep=0x1d29 wordp=0x60d7
// ================================================
// 0x60d7: db 0x43 0x59 'CY'

// ================================================
// 0x60d9: WORD ''CREW-C' codep=0x1d29 wordp=0x60e5
// ================================================
// 0x60e5: db 0x4c 0x45 'LE'

// ================================================
// 0x60e7: WORD ''EXTERN' codep=0x1d29 wordp=0x60f3
// ================================================
// 0x60f3: db 0x56 0xca 'V '

// ================================================
// 0x60f5: WORD ''REPAIR' codep=0x1d29 wordp=0x6101
// ================================================
// 0x6101: db 0x74 0x65 'te'

// ================================================
// 0x6103: WORD ''TREATM' codep=0x1d29 wordp=0x610f
// ================================================
// 0x610f: db 0x54 0x20 'T '

// ================================================
// 0x6111: WORD 'WEAPON-' codep=0x1d29 wordp=0x611d
// ================================================
// 0x611d: db 0x52 0x45 'RE'

// ================================================
// 0x611f: WORD '^CRIT' codep=0x1d29 wordp=0x6129
// ================================================
// 0x6129: db 0x45 0x20 'E '

// ================================================
// 0x612b: WORD '?FLAT' codep=0x1d29 wordp=0x6135
// ================================================
// 0x6135: db 0x74 0x6f 'to'

// ================================================
// 0x6137: WORD '1ST' codep=0x1d29 wordp=0x613f
// ================================================
// 0x613f: db 0x66 0x20 'f '

// ================================================
// 0x6141: WORD '?MVT' codep=0x1d29 wordp=0x614a
// ================================================
// 0x614a: db 0x00 0x00 '  '

// ================================================
// 0x614c: WORD '?SUP' codep=0x1d29 wordp=0x6155
// ================================================
// 0x6155: db 0x00 0x00 '  '

// ================================================
// 0x6157: WORD 'E-USE' codep=0x1d29 wordp=0x6161
// ================================================
// 0x6161: db 0x4c 0x45 'LE'

// ================================================
// 0x6163: WORD ''ENERGY' codep=0x1d29 wordp=0x616f
// ================================================
// 0x616f: db 0x62 0xcb 'b '

// ================================================
// 0x6171: WORD '?SECURE' codep=0x1d29 wordp=0x617d
// ================================================
// 0x617d: db 0x00 0x00 '  '

// ================================================
// 0x617f: WORD ''STP' codep=0x1d29 wordp=0x6188
// ================================================
// 0x6188: db 0x26 0xd2 '& '

// ================================================
// 0x618a: WORD ''RSIDE' codep=0x1d29 wordp=0x6195
// ================================================
// 0x6195: db 0x35 0xd2 '5 '

// ================================================
// 0x6197: WORD 'DERROR' codep=0x1d29 wordp=0x61a2
// ================================================
// 0x61a2: db 0x65 0x20 'e '

// ================================================
// 0x61a4: WORD 'NLR' codep=0x1d29 wordp=0x61ac
// ================================================
// 0x61ac: db 0x00 0x00 '  '

// ================================================
// 0x61ae: WORD 'OVT' codep=0x53f7 wordp=0x61b6
// ================================================
// 0x61b6: db 0xe6 0x07 0xe0 0x07 0xfa 0x07 0x47 0x57 0x48 0x5a 0x3d 0x5a 0x6c 0x5a 0x61 0x5a 0x77 0x5a 0x84 0x5a 0x56 0x5a '      GWHZ=ZlZaZwZ ZVZ'

// ================================================
// 0x61cc: WORD '?10' codep=0x1d29 wordp=0x61d4
// ================================================
// 0x61d4: db 0x00 0x00 '  '

// ================================================
// 0x61d6: WORD 'BITS' codep=0x1d29 wordp=0x61df
// ================================================
// 0x61df: db 0x50 0x03 'P '

// ================================================
// 0x61e1: WORD '#BITS' codep=0x1d29 wordp=0x61eb
// ================================================
// 0x61eb: db 0x10 0x00 '  '

// ================================================
// 0x61ed: WORD '?5' codep=0x1d29 wordp=0x61f4
// ================================================
// 0x61f4: db 0x00 0x00 '  '

// ================================================
// 0x61f6: WORD 'SELLING' codep=0x1d29 wordp=0x6202
// ================================================
// 0x6202: db 0x20 0x2d ' -'

// ================================================
// 0x6204: WORD 'BARTERI' codep=0x1d29 wordp=0x6210
// ================================================
// 0x6210: db 0x20 0x49 ' I'

// ================================================
// 0x6212: WORD '?REPLY' codep=0x1d29 wordp=0x621d
// ================================================
// 0x621d: db 0x52 0x41 'RA'

// ================================================
// 0x621f: WORD 'PLAST' codep=0x1d29 wordp=0x6229
// ================================================
// 0x6229: db 0x20 0x72 ' r'

// ================================================
// 0x622b: WORD 'PAST' codep=0x1d29 wordp=0x6234
// ================================================
// 0x6234: db 0x00 0x00 '  '

// ================================================
// 0x6236: WORD 'HAZE' codep=0x1d29 wordp=0x623f
// ================================================
// 0x623f: db 0x00 0x00 0x00 0x00 '    '

// ================================================
// 0x6243: WORD '?CALLIN' codep=0x1d29 wordp=0x624f
// ================================================
// 0x624f: db 0x01 0x00 '  '

// ================================================
// 0x6251: WORD 'STAGES' codep=0x1d29 wordp=0x625c
// ================================================
// 0x625c: db 0x63 0x65 0x05 0x41 0x4c 0x4c 0x4f 0x54 0x20 0x65 0x64 0x20 0x0d 0x0a 'ce ALLOT ed   '

// ================================================
// 0x626a: WORD '%SLUG' codep=0x1d29 wordp=0x6274
// ================================================
// 0x6274: db 0x64 0x00 'd '

// ================================================
// 0x6276: WORD 'ITEM' codep=0x1d29 wordp=0x627f
// ================================================
// 0x627f: db 0xeb 0x63 0x02 0x6c ' c l'

// ================================================
// 0x6283: WORD 'FSTUN' codep=0x1d29 wordp=0x628d
// ================================================
// 0x628d: db 0x00 0x00 '  '

// ================================================
// 0x628f: WORD 'PATIENC' codep=0x1d29 wordp=0x629b
// ================================================
// 0x629b: db 0x64 0x20 'd '

// ================================================
// 0x629d: WORD 'TMAP' codep=0x1d29 wordp=0x62a6
// ================================================
// 0x62a6: db 0x6f 0x72 0x20 0x66 'or f'

// ================================================
// 0x62aa: WORD ':(' codep=0x1d29 wordp=0x62b1
// ================================================
// 0x62b1: db 0x42 0xc3 'B '

// ================================================
// 0x62b3: WORD 'TIRED-T' codep=0x1d29 wordp=0x62bf
// ================================================
// 0x62bf: db 0x45 0x20 0x28 0x73 'E (s'

// ================================================
// 0x62c3: WORD 'LASTREP' codep=0x1d29 wordp=0x62cf
// ================================================
// 0x62cf: db 0x00 0x00 0x00 0x00 '    '

// ================================================
// 0x62d3: WORD 'TALKCOU' codep=0x1d29 wordp=0x62df
// ================================================
// 0x62df: db 0x20 0x69 0x74 0x69 ' iti'

// ================================================
// 0x62e3: WORD 'VSTIME' codep=0x1d29 wordp=0x62ee
// ================================================
// 0x62ee: db 0x00 0x00 0x00 0x00 '    '

// ================================================
// 0x62f2: WORD '10*CARG' codep=0x1d29 wordp=0x62fe
// ================================================
// 0x62fe: db 0x00 0x00 0xb0 0x36 '   6'

// ================================================
// 0x6302: WORD 'SENSE-A' codep=0x1d29 wordp=0x630e
// ================================================
// 0x630e: db 0x00 0x00 0x00 0x29 '   )'

// ================================================
// 0x6312: WORD 'EYEXY' codep=0x1d29 wordp=0x631c
// ================================================
// 0x631c: db 0x52 0x20 0x64 0x64 'R dd'

// ================================================
// 0x6320: WORD 'WEAPXY' codep=0x1d29 wordp=0x632b
// ================================================
// 0x632b: db 0x74 0x20 0x53 0x45 't SE'

// ================================================
// 0x632f: WORD '10*END' codep=0x1d29 wordp=0x633a
// ================================================
// 0x633a: db 0x00 0x00 0x98 0x3a '   :'

// ================================================
// 0x633e: WORD 'TOWFINE' codep=0x1d29 wordp=0x634a
// ================================================
// 0x634a: db 0x16 0x41 0x63 0x74 ' Act'

// ================================================
// 0x634e: WORD 'ENC-TIM' codep=0x1d29 wordp=0x635a
// ================================================
// 0x635a: db 0x12 0x00 0x23 0x17 '  # '

// ================================================
// 0x635e: WORD 'NAV-TIM' codep=0x1d29 wordp=0x636a
// ================================================
// 0x636a: db 0x6e 0x6f 0x74 0x20 'not '

// ================================================
// 0x636e: WORD 'W6370' codep=0x1d29 wordp=0x6370
// ================================================
// 0x6370: db 0xbf 0x05 0xe3 0x31 '   1'

// ================================================
// 0x6374: WORD 'STIME' codep=0x1d29 wordp=0x637e
// ================================================
// 0x637e: db 0x65 0x6e 0x74 0x20 'ent '

// ================================================
// 0x6382: WORD 'ETIME' codep=0x1d29 wordp=0x638c
// ================================================
// 0x638c: db 0x00 0x00 '  '

// ================================================
// 0x638e: WORD 'W6390' codep=0x1d29 wordp=0x6390
// ================================================
// 0x6390: db 0x03 0x00 0x40 0xc6 '  @ '

// ================================================
// 0x6394: WORD 'W6396' codep=0x1d29 wordp=0x6396
// ================================================
// 0x6396: db 0x03 0x00 0x40 0xc6 '  @ '

// ================================================
// 0x639a: WORD 'W639C' codep=0x1d29 wordp=0x639c
// ================================================
// 0x639c: db 0x00 0x00 0x00 0x00 '    '

// ================================================
// 0x63a0: WORD 'W63A2' codep=0x1d29 wordp=0x63a2
// ================================================
// 0x63a2: db 0xe5 0xbf 0x01 0x72 '   r'

// ================================================
// 0x63a6: WORD 'KEYTIME' codep=0x1d29 wordp=0x63b2
// ================================================
// 0x63b2: db 0x74 0x20 0x6f 0x66 't of'

// ================================================
// 0x63b6: WORD 'LKEYTIM' codep=0x1d29 wordp=0x63c2
// ================================================
// 0x63c2: db 0x69 0x63 0x68 0x20 'ich '

// ================================================
// 0x63c6: WORD '(SCROLL_1' codep=0x1d29 wordp=0x63d2
// ================================================
// 0x63d2: db 0x4f 0x58 0x29 0x20 'OX) '

// ================================================
// 0x63d6: WORD '(ORIGIN' codep=0x1d29 wordp=0x63e2
// ================================================
// 0x63e2: db 0x4f 0x52 0x29 0x20 'OR) '

// ================================================
// 0x63e6: WORD '(SCROLL_2' codep=0x1d29 wordp=0x63f2
// ================================================
// 0x63f2: db 0x4f 0x4e 0x54 0x29 'ONT)'

// ================================================
// 0x63f6: WORD 'REAL-MS' codep=0x1d29 wordp=0x6402
// ================================================
// 0x6402: db 0x00 0x00 0x10 0x27 '   ''

// ================================================
// 0x6406: WORD 'LAST-UP' codep=0x1d29 wordp=0x6412
// ================================================
// 0x6412: db 0x00 0x00 0x00 0x00 '    '

// ================================================
// 0x6416: WORD 'XWLD:XP' codep=0x1d29 wordp=0x6422
// ================================================
// 0x6422: db 0x20 0x00 0x08 0x00 '    '

// ================================================
// 0x6426: WORD 'YWLD:YP' codep=0x1d29 wordp=0x6432
// ================================================
// 0x6432: db 0x30 0x00 0x08 0x00 '0   '

// ================================================
// 0x6436: WORD 'ANCHOR' codep=0x1d29 wordp=0x6441
// ================================================
// 0x6441: db 0x00 0x00 0x4a 0x00 '  J '

// ================================================
// 0x6445: WORD 'OK-TALK' codep=0x1d29 wordp=0x6451
// ================================================
// 0x6451: db 0x49 0x4d 0x45 0x20 'IME '

// ================================================
// 0x6455: WORD 'TVEHICL' codep=0x1d29 wordp=0x6461
// ================================================
// 0x6461: db 0xd4 0x2b 0x02 0x56 ' + V'

// ================================================
// 0x6465: WORD 'TV-HOLD' codep=0x1d29 wordp=0x6471
// ================================================
// 0x6471: db 0x00 0x2c 0x02 0x61 ' , a'

// ================================================
// 0x6475: WORD 'SUPER-B' codep=0x1d29 wordp=0x6481
// ================================================
// 0x6481: db 0xaf 0x10 0x00 0x61 '   a'

// ================================================
// 0x6485: WORD '(SYSTEM' codep=0x1d29 wordp=0x6491
// ================================================
// 0x6491: db 0xd1 0xbf 0x01 0x65 '   e'

// ================================================
// 0x6495: WORD '(ORBIT)' codep=0x1d29 wordp=0x64a1
// ================================================
// 0x64a1: db 0xd1 0x67 0x02 0x65 ' g e'

// ================================================
// 0x64a5: WORD '(PLANET' codep=0x1d29 wordp=0x64b1
// ================================================
// 0x64b1: db 0xe5 0xbf 0x01 0x65 '   e'

// ================================================
// 0x64b5: WORD '(SURFAC' codep=0x1d29 wordp=0x64c1
// ================================================
// 0x64c1: db 0x9c 0x10 0x00 0x65 '   e'

// ================================================
// 0x64c5: WORD '(ENCOUN' codep=0x1d29 wordp=0x64d1
// ================================================
// 0x64d1: db 0x00 0x00 0x00 0x00 '    '

// ================================================
// 0x64d5: WORD '(SHIPBO' codep=0x1d29 wordp=0x64e1
// ================================================
// 0x64e1: db 0x48 0x2b 0x02 0x65 'H+ e'

// ================================================
// 0x64e5: WORD '(AORIGI' codep=0x1d29 wordp=0x64f1
// ================================================
// 0x64f1: db 0x00 0x00 0x00 0x00 '    '

// ================================================
// 0x64f5: WORD 'THIS-RE' codep=0x1d29 wordp=0x6501
// ================================================
// 0x6501: db 0x4f 0x4e 0x20 0x6e 'ON n'

// ================================================
// 0x6505: WORD '(THIS-I' codep=0x1d29 wordp=0x6511
// ================================================
// 0x6511: db 0x4d 0x29 0x20 0x68 'M) h'

// ================================================
// 0x6515: WORD 'W6517' codep=0x1d29 wordp=0x6517
// ================================================
// 0x6517: db 0x45 0xb2 'E '

// ================================================
// 0x6519: WORD 'W651B' codep=0x1d29 wordp=0x651b
// ================================================
// 0x651b: db 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xb2 0x17 0x00 0x32 0x7d 0x02 0xb2 0x17 0x00 0xba 0x6c 0x02 0x48 0x2b 0x02 0xd1 0x67 0x02 0x48 0x2b 0x02 0xd1 0x67 0x02 0xd1 0xbf 0x01 0x0e 0x11 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x29 0x1d 0x3a 0x20 0x01 0x00 0x96 0xc9 0x01 0x00 0xc3 0xc9 0x01 0x00 0xa5 0xc9 0x01 0x00 0xe1 0xc9 0x01 0x00 0x2c 0xca 0x01 0x00 0x77 0xca 0x01 0x00 0xb3 0xca 0x01 0x00 0xe0 0xca 0x01 0x00 0x0d 0xcb 0x01 0x00 0x67 0xcb 0x01 0x00 0x01 0x00 0xb3 0x7a 0x00 0x00 0x48 0x3a 0x09 0x76 0x01 0x00 0xb3 0x7a 0xec 0x7d 0x00 0x00 0x6e 0x00 '                                                                                       2}     l H+  g H+  g             ) :                   ,   w               g      z  H: v   z }  n '

// ================================================
// 0x65d5: WORD 'IBFR' codep=0x1d29 wordp=0x65de
// ================================================
// 0x65de: db 0x0e 0x00 0x00 0x00 0x2b 0x02 0x25 0x2b 0x02 0x7c 0x2b 0x02 0x12 0x00 0x08 0x00 0x7b 0xff 0xa8 0xfe 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x20 0x0b 0x41 0x2b 0xaa 0xd8 0x07 0xac 0xeb 0xab 0x5a 0xb4 0xac 0x36 0x8f 0x1f 0x61 0x14 0x20 0x57 0x49 0x4c 0x4c 0x20 0x42 0x45 0x20 0x4d 0x45 0x54 0x20 0x57 0x49 0x54 0x48 0x20 0x46 0x4f 0x52 0x43 0x45 0x2e 0x20 0x44 0x4f 0x20 0x59 0x4f 0x55 0x20 0x41 0x47 0x52 0x45 0x45 0x20 0x54 0x4f 0x20 0x43 0x4f 0x4d 0x45 0x20 0x41 0x4c 0x4f 0x4e 0x47 0x20 0x50 0x45 0x41 0x43 0x45 0x46 0x55 0x4c 0x4c 0x59 0x3f 0x20 0x20 0x25 0x31 0x0e 0x07 0x8d 0x3e 0x34 0x58 0x83 0xc1 0x5e 0xc3 0x02 0x04 0x00 0x01 0xff 0xff 0x04 0x00 0x0e 0x50 0x61 0x72 0x73 0x65 0x20 0x45 0x72 0x72 0x6f 0x72 0x20 0x25 0x31 0x0e 0x07 0x8d 0x3e 0x96 0x58 0x83 0xc1 0x1d 0xc3 0xff 0x00 0x00 0x00 0x00 0x0b 0x00 0x16 '    + %+ |+     {                                                                                                                          A+      Z  6  a  WILL BE MET WITH FORCE. DO YOU AGREE TO COME ALONG PEACEFULLY?  %1   >4X  ^          Parse Error %1   > X            '

// ================================================
// 0x66ef: WORD 'LSCAN' codep=0x1d29 wordp=0x66f9
// ================================================
// 0x66f9: db 0x50 0x4f 0x4c 0x59 0x43 0x4f 0x4e 0x20 0x20 0x20 0x20 0x20 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x03 0x9c 0x05 0x01 0x20 0x00 0x04 0x01 0x0b 0x00 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x5c 0x9c 0x01 0x00 0x80 0x01 0x20 0x12 0x04 0x02 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x53 0x9c 0x30 0x3c 0x30 0x3c 0x30 0x3c 0x30 0x3c 0x30 0x3c 0x04 0x01 'POLYCON                                                                                                         \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \         S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S 0<0<0<0<0<  '

// ================================================
// 0x6889: WORD 'W688B' codep=0x1d29 wordp=0x688b
// ================================================
// 0x688b: db 0xc2 0x00 0x3c 0x00 0xc2 0x00 0x30 0x00 0xc6 0x00 0x30 0x00 0xc6 0x00 0x3c 0x00 0x49 0x41 0x42 0x4c 0x45 0x20 0x2d 0x20 0x41 0x52 0x52 0x41 0x59 0x20 0x2d 0x20 0x53 0x48 0x5c 0x20 0x56 0x41 0x52 0x20 0x20 0x20 0x20 0x20 0x4b 0x52 0x4e 0x20 0x20 0x72 0x66 0x67 0x32 0x32 0x61 0x75 0x67 0x38 0x39 0x20 0x29 0x2e 0x04 0x00 '  <   0   0   < IABLE - ARRAY - SH\ VAR     KRN  rfg22aug89 ).  '

// ================================================
// 0x68cb: WORD 'W68CD' codep=0x1d29 wordp=0x68cd
// ================================================
// 0x68cd: db 0x3a 0x20 0x05 0x41 0x4c 0x4c 0x4f 0x54 0x20 0x02 0x0b 0x00 0x92 0x63 0x00 0x00 0x01 0x10 0x03 0x03 0x20 0x0b 0x00 0x00 0x00 0x00 0x00 0x02 0x34 0x0a 0x08 0x20 0x34 0x04 0x01 0x0b 0x00 0x00 0x00 0x00 0x00 0x01 0xa6 0x0b 0x0b 0x20 0x35 0x04 0x02 0x0b 0x00 0x00 0x00 0x00 0x00 0x01 0xa4 0x0a 0x08 0x20 0x0b 0x00 0x00 0x00 ':  ALLOT     c              4   4             5                 '

// ================================================
// 0x690d: WORD 'W690F' codep=0x1d29 wordp=0x690f
// ================================================
// 0x690f: db 0x3a 0x20 0x05 0x41 0x4c 0x4c 0x4f 0x54 0x20 0x41 0x54 0x48 0x3d 0x50 0x52 0x4f 0x4d 0x50 0x54 0x3d 0x43 0x4f 0x4d 0x53 0x50 0x45 0x43 0x3d 0x04 0x1b 0x5b 0x32 0x4a 0x42 0xa8 0x16 0x44 0xad 0x2f 0x45 0x9c 0x16 0x47 0xa0 0x16 0x48 0x92 0x16 0x4c 0xa4 0x16 0x4e 0xb9 0x16 0x50 0xc3 0x16 0x51 0x98 0x16 0x54 0x40 0x28 0x56 ':  ALLOT ATH=PROMPT=COMSPEC=  [2JB  D /E  G  H  L  N  P  Q  T@(V'

// ================================================
// 0x694f: WORD 'W6951' codep=0x1d29 wordp=0x6951
// ================================================
// 0x6951: db 0x3a 0x20 0x05 0x41 0x4c 0x4c 0x4f 0x54 0x20 0x4f 0x54 0x4e 0x0a 0x0a 0x45 0x52 0x52 0x4f 0x52 0x4c 0x45 0x56 0x45 0x4c 0x12 0x0b 0x05 0x45 0x58 0x49 0x53 0x54 0xa5 0x0a 0x00 0x03 0x44 0x49 0x52 0x03 0xaf 0x0f 0x04 0x43 0x41 0x4c 0x4c 0x02 0x87 0x0b 0x04 0x43 0x48 0x43 0x50 0x02 0x0d 0x19 0x06 0x52 0x45 0x4e 0x41 0x4d ':  ALLOT OTN  ERRORLEVEL   EXIST    DIR    CALL    CHCP    RENAM'

// ================================================
// 0x6991: WORD 'CMAP' codep=0x1d29 wordp=0x699a
// ================================================
// 0x699a: db 0x01 0x01 0x01 0x01 0x01 0x01 0x01 0x01 0x02 0x02 0x02 0x02 0x02 0x02 0x02 0x02 0x03 0x03 0x03 0x03 0x03 0x03 0x03 0x03 0x0e 0x0e 0x0e 0x0e 0x0e 0x0e 0x0e 0x0e 0x06 0x06 0x06 0x06 0x06 0x06 0x06 0x06 0x08 0x08 0x08 0x08 0x08 0x08 0x08 0x08 0x07 0x07 0x07 0x07 0x07 0x07 0x07 0x07 0x0f 0x0f 0x0f 0x0f 0x0f 0x0f 0x0f 0x0f '                                                                '

// ================================================
// 0x69da: WORD 'ATIME' codep=0x1d29 wordp=0x69e4
// ================================================
// 0x69e4: db 0x70 0x20 0x61 0x72 'p ar'

// ================================================
// 0x69e8: WORD 'LRTRN' codep=0x1d29 wordp=0x69f2
// ================================================
// 0x69f2: db 0x48 0x44 0x49 0x52 'HDIR'

// ================================================
// 0x69f6: WORD '(TRADER' codep=0x1d29 wordp=0x6a02
// ================================================
// 0x6a02: db 0xec 0x13 0x00 0x03 '    '

// ================================================
// 0x6a06: WORD '(THING)' codep=0x1d29 wordp=0x6a12
// ================================================
// 0x6a12: db 0x49 0x52 0x03 0x46 'IR F'

// ================================================
// 0x6a16: WORD 'BUY-LIS' codep=0x3b74 wordp=0x6a22
// ================================================
// 0x6a22: dw 0x0000 0x1063

// ================================================
// 0x6a26: WORD '#ETIME' codep=0x1d29 wordp=0x6a31
// ================================================
// 0x6a31: db 0x06 0x50 0x52 0x4f ' PRO'

// ================================================
// 0x6a35: WORD 'NTIME' codep=0x1d29 wordp=0x6a3f
// ================================================
// 0x6a3f: db 0x00 0x00 0x00 0x00 '    '

// ================================================
// 0x6a43: WORD ''UHL' codep=0x1d29 wordp=0x6a4c
// ================================================
// 0x6a4c: db 0x48 0x3a 'H:'

// ================================================
// 0x6a4e: WORD 'MERCATO' codep=0x4e6f wordp=0x6a5a
// ================================================
ArrayType MERCATO = {0x0030, 0x0018, 0x0480, 0x9cd1};

// ================================================
// 0x6a62: WORD 'CONANCH' codep=0x4e6f wordp=0x6a6e
// ================================================
ArrayType CONANCH = {0x0009, 0x0007, 0x003f, 0x9ccc};

// ================================================
// 0x6a76: WORD 'CONTOUR' codep=0x4e6f wordp=0x6a82
// ================================================
ArrayType CONTOUR = {0x003d, 0x0065, 0x1811, 0x9b3e};

// ================================================
// 0x6a8a: WORD 'ICONIMA' codep=0x4e6f wordp=0x6a96
// ================================================
ArrayType ICONIMA = {0x0012, 0x0040, 0x0480, 0x9df2};

// ================================================
// 0x6a9e: WORD 'VERTEX' codep=0x4e6f wordp=0x6aa9
// ================================================
ArrayType VERTEX = {0x0003, 0x0086, 0x0192, 0x9fd5};

// ================================================
// 0x6ab1: WORD 'FACET' codep=0x4e6f wordp=0x6abb
// ================================================
ArrayType FACET = {0x0004, 0x0048, 0x0120, 0x9fb9};

// ================================================
// 0x6ac3: WORD 'FACE' codep=0x4e6f wordp=0x6acc
// ================================================
ArrayType FACE = {0x0003, 0x0048, 0x00d8, 0x9fa2};

// ================================================
// 0x6ad4: WORD 'PPOLY' codep=0x4e6f wordp=0x6ade
// ================================================
ArrayType PPOLY = {0x0005, 0x0320, 0x0fa0, 0x9e43};

// ================================================
// 0x6ae6: WORD 'GVERTEX' codep=0x4e6f wordp=0x6af2
// ================================================
ArrayType GVERTEX = {0x0003, 0x0075, 0x015f, 0x9dcd};

// ================================================
// 0x6afa: WORD 'GPOLY' codep=0x4e6f wordp=0x6b04
// ================================================
ArrayType GPOLY = {0x0004, 0x0059, 0x0164, 0x9dab};

// ================================================
// 0x6b0c: WORD 'GRIDCOL' codep=0x4e6f wordp=0x6b18
// ================================================
ArrayType GRIDCOL = {0x0002, 0x0059, 0x00b2, 0x9d94};

// ================================================
// 0x6b20: WORD 'G1VERT' codep=0x4e6f wordp=0x6b2b
// ================================================
ArrayType G1VERT = {0x0003, 0x0075, 0x015f, 0x9d6f};

// ================================================
// 0x6b33: WORD '9X9COAR' codep=0x4e6f wordp=0x6b3f
// ================================================
ArrayType _9X9COAR = {0x0009, 0x0009, 0x0051, 0x9d68};

// ================================================
// 0x6b47: WORD '9X9FINE' codep=0x4e6f wordp=0x6b53
// ================================================
ArrayType _9X9FINE = {0x0009, 0x0009, 0x0051, 0x9d61};

// ================================================
// 0x6b5b: WORD 'XFORMVE' codep=0x4e6f wordp=0x6b67
// ================================================
ArrayType XFORMVE = {0x0006, 0x0086, 0x0324, 0x9d1d};

// ================================================
// 0x6b6f: WORD 'IARRAYS' codep=0x53f7 wordp=0x6b7b
// ================================================
// 0x6b7b: db 0xa9 0x6a 0xbb 0x6a 0xcc 0x6a 0xde 0x6a 0x96 0x6a 0xf2 0x6a 0x04 0x6b 0x18 0x6b 0x2b 0x6b 0x3f 0x6b 0x53 0x6b 0x67 0x6b 0x5a 0x6a 0x6e 0x6a 0x82 0x6a ' j j j j j j k k+k?kSkgkZjnj j'

// ================================================
// 0x6b99: WORD '*STARPO' codep=0x3b74 wordp=0x6ba5
// ================================================
// 0x6ba5: dw 0x0002 0x2784

// ================================================
// 0x6ba9: WORD '*PERSON' codep=0x3b74 wordp=0x6bb5
// ================================================
// 0x6bb5: dw 0x0002 0x27a3

// ================================================
// 0x6bb9: WORD '*SHIP-C' codep=0x3b74 wordp=0x6bc5
// ================================================
// 0x6bc5: dw 0x0002 0x2b48

// ================================================
// 0x6bc9: WORD '*TRADE-' codep=0x3b74 wordp=0x6bd5
// ================================================
// 0x6bd5: dw 0x0002 0x2b53

// ================================================
// 0x6bd9: WORD '*ITEMS-' codep=0x3b74 wordp=0x6be5
// ================================================
// 0x6be5: dw 0x0002 0x2b5e

// ================================================
// 0x6be9: WORD '*OPERAT' codep=0x3b74 wordp=0x6bf5
// ================================================
// 0x6bf5: dw 0x0002 0x2b71

// ================================================
// 0x6bf9: WORD '*BANK' codep=0x3b74 wordp=0x6c03
// ================================================
// 0x6c03: dw 0x0002 0x278f

// ================================================
// 0x6c07: WORD '*STARSH' codep=0x3b74 wordp=0x6c13
// ================================================
// 0x6c13: dw 0x0002 0x2be6

// ================================================
// 0x6c17: WORD '*EYE' codep=0x3b74 wordp=0x6c20
// ================================================
// 0x6c20: dw 0x0002 0x03d3

// ================================================
// 0x6c24: WORD '*PLIST' codep=0x3b74 wordp=0x6c2f
// ================================================
// 0x6c2f: dw 0x0000 0x10c2

// ================================================
// 0x6c33: WORD '*MISS' codep=0x3b74 wordp=0x6c3d
// ================================================
// 0x6c3d: dw 0x0000 0x10d5

// ================================================
// 0x6c41: WORD '*MESS' codep=0x3b74 wordp=0x6c4b
// ================================================
// 0x6c4b: dw 0x0000 0x10fb

// ================================================
// 0x6c4f: WORD ''VERSIO' codep=0x1d29 wordp=0x6c5b
// ================================================
// 0x6c5b: db 0x1d 0xaa '  '

// ================================================
// 0x6c5d: WORD 'CTTOP' codep=0x1d29 wordp=0x6c67
// ================================================
// 0x6c67: db 0x32 0x00 '2 '

// ================================================
// 0x6c69: WORD 'CTBOT' codep=0x1d29 wordp=0x6c73
// ================================================
// 0x6c73: db 0x02 0x00 '  '

// ================================================
// 0x6c75: WORD '>0FONT_1' codep=0x224c wordp=0x6c80 params=0 returns=0
// ================================================

void _gt_0FONT_1() // >0FONT_1
{
  Push(0x2731); // '(EMIT)'
  Push(0x066d); // 'EMIT'
  EXECUTES(); // EXECUTES
  Push(0x2690); // '(TYPE)'
  Push(0x06e8); // 'TYPE'
  EXECUTES(); // EXECUTES
  Push(0x26ee); // '(CR)'
  Push(0x0644); // 'CR'
  EXECUTES(); // EXECUTES
  Push(0);
  Push(pp_FONT_n_); // FONT#
  Store(); // !
}


// ================================================
// 0x6ca6: WORD '?UPDATE' codep=0x6cb2 wordp=0x6cb2 params=1 returns=1
// ================================================
// 0x6cb2: pop    cx
// 0x6cb3: or     cx,cx
// 0x6cb5: jns    6CED
// 0x6cb7: mov    bx,[5625] // W5625
// 0x6cbb: mov    dx,bx
// 0x6cbd: add    dx,07
// 0x6cc0: cmp    cx,dx
// 0x6cc2: jle    6CEB
// 0x6cc4: add    dx,0401
// 0x6cc8: cmp    dx,cx
// 0x6cca: jle    6CD2
// 0x6ccc: mov    byte ptr [bx+02],FF
// 0x6cd0: jmp    6CEB
// 0x6cd2: mov    bx,[5629] // W5629
// 0x6cd6: mov    dx,bx
// 0x6cd8: add    dx,07
// 0x6cdb: cmp    cx,dx
// 0x6cdd: jle    6CEB
// 0x6cdf: add    dx,0401
// 0x6ce3: cmp    dx,cx
// 0x6ce5: jle    6CEB
// 0x6ce7: mov    byte ptr [bx+02],FF
// 0x6ceb: jmp    6CFF
// 0x6ced: cmp    cx,65E1
// 0x6cf1: js     6CFF
// 0x6cf3: cmp    cx,66EF
// 0x6cf7: jns    6CFF
// 0x6cf9: mov    bx,65E0
// 0x6cfc: mov    byte ptr [bx],FF
// 0x6cff: push   cx
// 0x6d00: lodsw
// 0x6d01: mov    bx,ax
// 0x6d03: jmp    word ptr [bx]

// ================================================
// 0x6d05: WORD 'C!_2' codep=0x224c wordp=0x6d0c params=2 returns=0
// ================================================

void C_ex__2() // C!_2
{
  IsUPDATE(); // ?UPDATE
  C_ex__1(); // C!_1
}


// ================================================
// 0x6d12: WORD '!_2' codep=0x224c wordp=0x6d18 params=2 returns=0
// ================================================

void Store_2() // !_2
{
  IsUPDATE(); // ?UPDATE
  Store(); // !
}


// ================================================
// 0x6d1e: WORD '+!_2' codep=0x224c wordp=0x6d25 params=2 returns=0
// ================================================

void _plus__ex__2() // +!_2
{
  IsUPDATE(); // ?UPDATE
  _plus__ex_(); // +!
}


// ================================================
// 0x6d2b: WORD '1.5!_2' codep=0x224c wordp=0x6d34 params=3 returns=0
// ================================================

void _1_dot_5_ex__2() // 1.5!_2
{
  IsUPDATE(); // ?UPDATE
  _1_dot_5_ex__1(); // 1.5!_1
}


// ================================================
// 0x6d3a: WORD '2!_2' codep=0x224c wordp=0x6d41 params=3 returns=0
// ================================================

void _2_ex__2() // 2!_2
{
  IsUPDATE(); // ?UPDATE
  _2_ex__1(); // 2!_1
}


// ================================================
// 0x6d47: WORD 'D!' codep=0x224c wordp=0x6d4e params=3 returns=0
// ================================================

void StoreD() // D!
{
  _2_ex__2(); // 2!_2
}


// ================================================
// 0x6d52: WORD 'ON_2' codep=0x224c wordp=0x6d59 params=1 returns=0
// ================================================

void ON_2() // ON_2
{
  IsUPDATE(); // ?UPDATE
  ON_1(); // ON_1
}


// ================================================
// 0x6d5f: WORD '099' codep=0x224c wordp=0x6d67 params=1 returns=0
// ================================================

void _099() // 099
{
  IsUPDATE(); // ?UPDATE
  OFF_2(); // OFF_2
}


// ================================================
// 0x6d6d: WORD '2OFF' codep=0x224c wordp=0x6d76 params=1 returns=0
// ================================================

void _2OFF() // 2OFF
{
  IsUPDATE(); // ?UPDATE
  Push(0);
  Push(0);
  ROT(); // ROT
  _2_ex__2(); // 2!_2
}


// ================================================
// 0x6d82: WORD 'CMOVE_2' codep=0x224c wordp=0x6d8c params=3 returns=0
// ================================================

void CMOVE_2() // CMOVE_2
{
  OVER(); // OVER
  IsUPDATE(); // ?UPDATE
  Pop(); // DROP
  CMOVE_1(); // CMOVE_1
}


// ================================================
// 0x6d96: WORD 'FILL_2' codep=0x224c wordp=0x6d9f params=3 returns=0
// ================================================

void FILL_2() // FILL_2
{
  Push(3);
  PICK(); // PICK
  IsUPDATE(); // ?UPDATE
  Pop(); // DROP
  FILL_1(); // FILL_1
}


// ================================================
// 0x6dab: WORD 'W6DAD' codep=0x224c wordp=0x6dad params=0 returns=0
// ================================================

void W6DAD() // W6DAD
{
  Push(pp_W55FF); // W55FF
  OFF_2(); // OFF_2
  Push(pp_W5603); // W5603
  OFF_2(); // OFF_2
}


// ================================================
// 0x6db7: WORD 'BLOCK_2' codep=0x224c wordp=0x6dc1 params=1 returns=2
// ================================================

void BLOCK_2() // BLOCK_2
{
  W6DAD(); // W6DAD
  BLOCK_1(); // BLOCK_1
}


// ================================================
// 0x6dc7: WORD 'LBLOCK_2' codep=0x224c wordp=0x6dd2 params=1 returns=2
// ================================================

void LBLOCK_2() // LBLOCK_2
{
  W6DAD(); // W6DAD
  LBLOCK_1(); // LBLOCK_1
}


// ================================================
// 0x6dd8: WORD 'LOAD_2' codep=0x224c wordp=0x6de1
// ================================================

void LOAD_2() // LOAD_2
{
  Push(Read16(regsp)); // DUP
  Push(user_SCR); // SCR
  Store_2(); // !_2
  W6DAD(); // W6DAD
  Exec("LOAD_1"); // call of word 0x1e23 '(LOAD)'
}


// ================================================
// 0x6ded: WORD '-TEXT' codep=0x224c wordp=0x6df7 params=3 returns=1
// ================================================

void _dash_TEXT() // -TEXT
{
  unsigned short int i, imax;
  Push(0);
  ROT(); // ROT

  i = 0;
  imax = Pop();
  do // (DO)
  {
    Pop(); // DROP
    OVER(); // OVER
    Push(Read16(Pop() + i)&0xFF); //  I + C@
    OVER(); // OVER
    Push(Read16(Pop() + i)&0xFF); //  I + C@
    _dash_(); // -
    Push(Read16(regsp)); // DUP
    Push(!(Pop()==0?1:0)); //  0= NOT
    if (Pop() != 0)
    {
      imax = i; // LEAVE
    }
    i++;
  } while(i<imax); // (LOOP)

  SWAP(); // SWAP
  Pop(); // DROP
  SWAP(); // SWAP
  Pop(); // DROP
}


// ================================================
// 0x6e2d: WORD '$=' codep=0x224c wordp=0x6e34 params=2 returns=1
// ================================================

void _do__eq_() // $=
{
  _2DUP(); // 2DUP
  Push(Read16(Pop())&0xFF); //  C@
  SWAP(); // SWAP
  Push(Read16(Pop())&0xFF); //  C@
  Push((Pop()==Pop())?1:0); // =
  if (Pop() != 0)
  {
    COUNT(); // COUNT
    ROT(); // ROT
    Push(Pop() + 1); //  1+
    _dash_TEXT(); // -TEXT
    Push(!Pop()); //  NOT
    return;
  }
  Pop(); Pop(); // 2DROP
  Push(0);
}


// ================================================
// 0x6e56: WORD 'SIGFLD' codep=0x224c wordp=0x6e61
// ================================================

void SIGFLD() // SIGFLD
{
  Exec("CREATE"); // call of word 0x1cbb '(CREATE)'
  _co_(); // ,
  CODE(); // (;CODE) inlined assembler code
// 0x6e67: call   1649
  Push(Read16(Pop())); //  @
  Push(Read16(cc_SIGBLK)); // SIGBLK
  BLOCK_2(); // BLOCK_2
  Push(Pop() + Pop()); // +
}


// ================================================
// 0x6e74: WORD ':SIGNAT' codep=0x6e67 wordp=0x6e80
// ================================================
// 0x6e80: db 0xf0 0x03 '  '

// ================================================
// 0x6e82: WORD ':TIMEST' codep=0x6e67 wordp=0x6e8e
// ================================================
// 0x6e8e: db 0xf4 0x03 '  '

// ================================================
// 0x6e90: WORD ':CKSUM' codep=0x6e67 wordp=0x6e9b
// ================================================
// 0x6e9b: db 0xf6 0x03 '  '

// ================================================
// 0x6e9d: WORD ':SAVE' codep=0x6e67 wordp=0x6ea7
// ================================================
// 0x6ea7: db 0xf8 0x03 '  '

// ================================================
// 0x6ea9: WORD ':VERSIO' codep=0x6e67 wordp=0x6eb5
// ================================================
// 0x6eb5: db 0xfa 0x03 '  '

// ================================================
// 0x6eb7: WORD 'THRU_2' codep=0x224c wordp=0x6ec0
// ================================================

void THRU_2() // THRU_2
{
  unsigned short int i, imax;
  _2DUP(); // 2DUP
  Push((Pop()==Pop())?1:0); // =
  if (Pop() != 0)
  {
    Pop(); // DROP
    LOAD_2(); // LOAD_2
    return;
  }
  Push(Pop() + 1); //  1+
  SWAP(); // SWAP

  i = Pop();
  imax = Pop();
  do // (DO)
  {
    Push(i); // I
    Push(Read16(regsp)); // DUP
    Draw(); // .
    LOAD_2(); // LOAD_2
    i++;
  } while(i<imax); // (LOOP)

}


// ================================================
// 0x6ee4: WORD 'VA>BLK' codep=0x6eef wordp=0x6eef params=2 returns=2
// ================================================
// 0x6eef: pop    ax
// 0x6ef0: pop    dx
// 0x6ef1: mov    cx,000A
// 0x6ef4: shr    ax,1
// 0x6ef6: rcr    dx,1
// 0x6ef8: rcr    bx,1
// 0x6efa: loop   6EF4
// 0x6efc: mov    cx,0006
// 0x6eff: shr    bx,cl
// 0x6f01: push   bx
// 0x6f02: push   dx
// 0x6f03: lodsw
// 0x6f04: mov    bx,ax
// 0x6f06: jmp    word ptr [bx]

// ================================================
// 0x6f08: WORD 'VA>BUF' codep=0x224c wordp=0x6f13 params=2 returns=2
// ================================================

void VA_gt_BUF() // VA>BUF
{
  VA_gt_BLK(); // VA>BLK
  BLOCK_2(); // BLOCK_2
  Push(Pop() + Pop()); // +
}


// ================================================
// 0x6f1b: WORD 'W6F1D' codep=0x224c wordp=0x6f1d params=1 returns=2
// ================================================

void W6F1D() // W6F1D
{
  SIGFLD(":SIGNAT");
  _2_at_(); // 2@
  SWAP(); // SWAP
}


// ================================================
// 0x6f25: WORD 'W6F27' codep=0x224c wordp=0x6f27 params=0 returns=1
// ================================================

void W6F27() // W6F27
{
  Push(Read16(pp_OFFSET)==0x05dc?1:0); // OFFSET @ 0x05dc =
}


// ================================================
// 0x6f33: WORD 'W6F35' codep=0x224c wordp=0x6f35 params=1 returns=1
// ================================================
// orphan

void W6F35() // W6F35
{
  SIGFLD(":TIMEST");
  Push((Read16(Pop())==Read16(pp_TIMESTA)?1:0) | Read16(pp_RELAXTI)); //  @ TIMESTA @ = RELAXTI @ OR
}


// ================================================
// 0x6f47: WORD 'W6F49' codep=0x224c wordp=0x6f49 params=1 returns=1
// ================================================
// orphan

void W6F49() // W6F49
{
  Push(Read16(pp__rc__c__dash__co_601)); // ):-,601 @
  SIGFLD(":VERSIO");
  Push(Read16(Pop())); //  @
  Push((Pop()==Pop())?1:0); // =
}


// ================================================
// 0x6f55: WORD 'W6F57' codep=0x224c wordp=0x6f57
// ================================================

void W6F57() // W6F57
{
  Push(pp__i_VERSIO); // 'VERSIO
  GetEXECUTE(); // @EXECUTE
}


// ================================================
// 0x6f5d: WORD 'MOUNTFI' codep=0x224c wordp=0x6f69
// ================================================

void MOUNTFI() // MOUNTFI
{
  while(1)
  {
    _2DUP(); // 2DUP
    _gt_TIB(); // >TIB
    Push(pp_W5711); // W5711
    GetEXECUTE(); // @EXECUTE
    Push(0);
    if (Pop() == 0) break;

    _2DUP(); // 2DUP
    W6F57(); // W6F57
  }
  Pop(); Pop(); // 2DROP
}


// ================================================
// 0x6f83: WORD 'W6F85' codep=0x224c wordp=0x6f85
// ================================================

void W6F85() // W6F85
{
  SYSUTIL(); // SYSUTIL
  SETFCB(); // SETFCB
  Pop(); // DROP
  DR3(); // DR3
}


// ================================================
// 0x6f8f: WORD 'W6F91' codep=0x224c wordp=0x6f91
// ================================================

void W6F91() // W6F91
{
  SYSTEM(); // SYSTEM
  SETFCB(); // SETFCB
  Pop(); // DROP
  DR2(); // DR2
}


// ================================================
// 0x6f9b: WORD 'MOUNTA' codep=0x224c wordp=0x6fa6
// ================================================

void MOUNTA() // MOUNTA
{
  Push(0x6f85); // 'W6F85'
  Push(pp_W5711); // W5711
  Store_2(); // !_2
  SET_STR_AS_PARAM("STAR2A.COM");
  MOUNTFI(); // MOUNTFI
}


// ================================================
// 0x6fbf: WORD 'MOUNTB' codep=0x224c wordp=0x6fca
// ================================================

void MOUNTB() // MOUNTB
{
  Push(0x6f91); // 'W6F91'
  Push(pp_W5711); // W5711
  Store_2(); // !_2
  SET_STR_AS_PARAM("STAR2B.COM");
  MOUNTFI(); // MOUNTFI
}


// ================================================
// 0x6fe3: WORD 'W6FE5' codep=0x224c wordp=0x6fe5 params=3 returns=1
// ================================================

void W6FE5() // W6FE5
{
  unsigned short int a;
  ROT(); // ROT
  a = Pop(); // >R
  Push(a); // I
  SWAP(); // SWAP
  U_st_(); // U<
  SWAP(); // SWAP
  Push(a + 1); // R> 1+
  U_st_(); // U<
  Push(Pop() & Pop()); // AND
}


// ================================================
// 0x6ffb: WORD 'W6FFD' codep=0x224c wordp=0x6ffd
// ================================================

void W6FFD() // W6FFD
{
  W6F27(); // W6F27
  if (Pop() != 0)
  {
    MOUNTB(); // MOUNTB
    return;
  }
  MOUNTA(); // MOUNTA
}


// ================================================
// 0x700d: WORD 'W700F' codep=0x224c wordp=0x700f params=2 returns=0
// ================================================

void W700F() // W700F
{
  W6F1D(); // W6F1D
  Push(Pop() + 1); //  1+
  W6FE5(); // W6FE5
  Push(!Pop()); //  NOT
  if (Pop() == 0) return;
  W6FFD(); // W6FFD
}


// ================================================
// 0x701f: WORD 'W7021' codep=0x224c wordp=0x7021
// ================================================

void W7021() // W7021
{
  Exec("CR"); // call of word 0x26ee '(CR)'
  PRINT("Place disk with the correct", 27); // (.")
  Exec("CR"); // call of word 0x26ee '(CR)'
  Exec("TYPE"); // call of word 0x2690 '(TYPE)'
  PRINT(" file and press a", 17); // (.")
  Exec("CR"); // call of word 0x26ee '(CR)'
  PRINT("key when ready.", 15); // (.")
  KEY_2(); // KEY_2
  Pop(); // DROP
}


// ================================================
// 0x7073: WORD 'W7075' codep=0x224c wordp=0x7075 params=2 returns=2
// ================================================

void W7075() // W7075
{
  W6F1D(); // W6F1D
  Pop(); // DROP
  _dash_(); // -
  Push(0);
  D16_star_(); // D16*
}


// ================================================
// 0x7081: WORD 'W7083' codep=0x224c wordp=0x7083 params=6 returns=1
// ================================================

void W7083() // W7083
{
  unsigned short int a, b, c;
  a = Pop(); // >R
  b = Pop(); // >R
  Push(Read16(regsp)); // DUP
  W700F(); // W700F
  W7075(); // W7075
  _2DUP(); // 2DUP
  Push(pp_W6396); // W6396
  _st_D_ex__gt_(); // <D!>
  Push(b >> 4); // R> 16*
  Push(0);
  D_plus_(); // D+
  Push(pp_W6390); // W6390
  _st_D_ex__gt_(); // <D!>
  Push(pp_MEMSEG); // MEMSEG
  _st__ex__gt_(); // <!>
  Push(pp_MEMOFF); // MEMOFF
  OFF_2(); // OFF_2
  do
  {
    Push(Read16(pp_MEMSEG)); // MEMSEG @
    Push(Read16(pp_MEMOFF)); // MEMOFF @
    Push(pp_W6396); // W6396
    _2_at_(); // 2@
    VA_gt_BLK(); // VA>BLK
    OVER(); // OVER
    Push(0x0400);
    SWAP(); // SWAP
    _dash_(); // -
    Push(0);
    Push(pp_W6390); // W6390
    _2_at_(); // 2@
    Push(pp_W6396); // W6396
    _2_at_(); // 2@
    D_dash_(); // D-
    DMIN(); // DMIN
    Pop(); // DROP
    c = Pop(); // >R
    BLOCK_2(); // BLOCK_2
    Push(Pop() + Pop()); // +
    GetDS(); // @DS
    SWAP(); // SWAP
    Push(a); // I'
    if (Pop() != 0)
    {
      UPDATE(); // UPDATE
    } else
    {
      _2SWAP(); // 2SWAP
    }
    Push(c); // I
    LCMOVE(); // LCMOVE
    Push(c); // I
    Push(pp_MEMOFF); // MEMOFF
    _st__plus__ex__gt_(); // <+!>
    Push(c); // R>
    Push(0);
    Push(pp_W6396); // W6396
    _2_at_(); // 2@
    D_plus_(); // D+
    _2DUP(); // 2DUP
    Push(pp_W6396); // W6396
    _st_D_ex__gt_(); // <D!>
    Push(pp_W6390); // W6390
    _2_at_(); // 2@
    D_eq_(); // D=
  } while(Pop() == 0);
  Push(a); // R>
  Pop(); // DROP
}


// ================================================
// 0x7111: WORD 'MEM>DSK' codep=0x224c wordp=0x711d params=5 returns=1
// ================================================

void MEM_gt_DSK() // MEM>DSK
{
  Push(1);
  W7083(); // W7083
}


// ================================================
// 0x7123: WORD 'MEM<DSK' codep=0x224c wordp=0x712f params=5 returns=1
// ================================================

void MEM_st_DSK() // MEM<DSK
{
  Push(0);
  W7083(); // W7083
}


// ================================================
// 0x7135: WORD 'W7137' codep=0x7137 wordp=0x7137
// ================================================
// 0x7137: mov    bx,sp
// 0x7139: mov    ax,0400
// 0x713c: sub    ax,[bx+02]
// 0x713f: xor    dx,dx
// 0x7141: mov    cx,[bx+04]
// 0x7144: div    cx
// 0x7146: cmp    ax,[bx+06]
// 0x7149: jle    7155
// 0x714b: mov    ax,[bx+06]
// 0x714e: mul    cx
// 0x7150: add    [bx+02],ax
// 0x7153: jmp    7179
// 0x7155: sub    [bx+06],ax
// 0x7158: inc    word ptr [bx]
// 0x715a: mov    word ptr [bx+02],0000
// 0x715f: mov    ax,0400
// 0x7162: xor    dx,dx
// 0x7164: div    cx
// 0x7166: mov    cx,ax
// 0x7168: mov    ax,[bx+06]
// 0x716b: xor    dx,dx
// 0x716d: div    cx
// 0x716f: add    [bx],ax
// 0x7171: mov    ax,dx
// 0x7173: mul    word ptr [bx+04]
// 0x7176: mov    [bx+02],ax
// 0x7179: pop    ax
// 0x717a: pop    cx
// 0x717b: add    sp,04
// 0x717e: push   cx
// 0x717f: push   ax
// 0x7180: lodsw
// 0x7181: mov    bx,ax
// 0x7183: jmp    word ptr [bx]

// ================================================
// 0x7185: WORD 'W7187' codep=0x7187 wordp=0x7187
// ================================================
// 0x7187: pop    ax
// 0x7188: cmp    ax,0090
// 0x718b: js     7199
// 0x718d: mov    bx,[54DE] // DIRBLK
// 0x7191: add    bx,03
// 0x7194: sub    ax,0090
// 0x7197: jmp    71BD
// 0x7199: cmp    ax,0060
// 0x719c: js     71AA
// 0x719e: mov    bx,[54DE] // DIRBLK
// 0x71a2: add    bx,02
// 0x71a5: sub    ax,0060
// 0x71a8: jmp    71BD
// 0x71aa: cmp    ax,0030
// 0x71ad: js     71B9
// 0x71af: mov    bx,[54DE] // DIRBLK
// 0x71b3: inc    bx
// 0x71b4: sub    ax,0030
// 0x71b7: jmp    71BD
// 0x71b9: mov    bx,[54DE] // DIRBLK
// 0x71bd: mov    cx,0015
// 0x71c0: imul   cx
// 0x71c2: push   ax
// 0x71c3: push   bx
// 0x71c4: lodsw
// 0x71c5: mov    bx,ax
// 0x71c7: jmp    word ptr [bx]

// ================================================
// 0x71c9: WORD 'W71CB' codep=0x224c wordp=0x71cb
// ================================================

void W71CB() // W71CB
{
  Exec("CREATE"); // call of word 0x1cbb '(CREATE)'
  C_co_(); // C,
  CODE(); // (;CODE) inlined assembler code
// 0x71d1: call   1649
  Push(Read16(Pop())&0xFF); //  C@
  SWAP(); // SWAP
  W7187(); // W7187
  BLOCK_2(); // BLOCK_2
  Push(Pop() + Pop()); // +
  Push(Pop() + Pop()); // +
}


// ================================================
// 0x71e2: WORD 'FILE-NA' codep=0x71d1 wordp=0x71ee
// ================================================
// 0x71ee: db 0x00 ' '

// ================================================
// 0x71ef: WORD 'FILE-TY' codep=0x71d1 wordp=0x71fb
// ================================================
// 0x71fb: db 0x0c ' '

// ================================================
// 0x71fc: WORD 'FILE-ST' codep=0x71d1 wordp=0x7208
// ================================================
// 0x7208: db 0x0d ' '

// ================================================
// 0x7209: WORD 'FILE-EN' codep=0x71d1 wordp=0x7215
// ================================================
// 0x7215: db 0x0f ' '

// ================================================
// 0x7216: WORD 'FILE-#R' codep=0x71d1 wordp=0x7222
// ================================================
// 0x7222: db 0x11 ' '

// ================================================
// 0x7223: WORD 'FILE-RL' codep=0x71d1 wordp=0x722f
// ================================================
// 0x722f: db 0x13 ' '

// ================================================
// 0x7230: WORD 'FILE-SL' codep=0x71d1 wordp=0x723c
// ================================================
// 0x723c: db 0x14 ' '

// ================================================
// 0x723d: WORD 'W723F' codep=0x723f wordp=0x723f
// ================================================
// 0x723f: pop    ax
// 0x7240: sub    ax,3E80
// 0x7243: mov    cx,0006
// 0x7246: xor    dx,dx
// 0x7248: shr    ax,1
// 0x724a: rcr    dl,1
// 0x724c: loop   7248
// 0x724e: shl    dx,1
// 0x7250: shl    dx,1
// 0x7252: push   dx
// 0x7253: push   ax
// 0x7254: lodsw
// 0x7255: mov    bx,ax
// 0x7257: jmp    word ptr [bx]

// ================================================
// 0x7259: WORD 'FILE:' codep=0x224c wordp=0x7263
// ================================================

void FILE_c_() // FILE:
{
  unsigned short int a, b, i, imax;
  Push(pp_LSCAN); // LSCAN
  Push(0x000c);
  Push(Read16(cc_BL)); // BL
  FILL_2(); // FILL_2
  Push(Read16(cc_BL)); // BL
  Exec("WORD"); // call of word 0x1f06 '(WORD)'
  COUNT(); // COUNT
  Push(pp_LSCAN); // LSCAN
  SWAP(); // SWAP
  CMOVE_2(); // CMOVE_2
  Push(Read16(user_BLK)); // BLK @
  a = Pop(); // >R
  Push(Read16(user__gt_IN)); // >IN @
  b = Pop(); // >R
  Push(-1);

  i = 0;
  imax = 0x00a3;
  do // (DO)
  {
    Push(pp_LSCAN); // LSCAN
    Push(0x000c);
    Push(i); // I
    Func6("FILE-NA");
    _dash_TEXT(); // -TEXT
    Push(!Pop()); //  NOT
    if (Pop() != 0)
    {
      Pop(); // DROP
      Push(i); // I
      imax = i; // LEAVE
    }
    i++;
  } while(i<imax); // (LOOP)

  Push(Read16(regsp)); // DUP
  _0_st_(); // 0<
  IsUNRAVEL(); // ?UNRAVEL
  Push(b); // R>
  Push(user__gt_IN); // >IN
  _st__ex__gt_(); // <!>
  Push(a); // R>
  Push(user_BLK); // BLK
  _st__ex__gt_(); // <!>
}


// ================================================
// 0x72bf: WORD '>FILE' codep=0x224c wordp=0x72c9
// ================================================

void _gt_FILE() // >FILE
{
  Func6("FILE-ST");
  _2_at_(); // 2@
  SWAP(); // SWAP
  OVER(); // OVER
  _dash_(); // -
  Push(Pop() + 1); //  1+
  MEM_gt_DSK(); // MEM>DSK
}


// ================================================
// 0x72d9: WORD 'FILE<' codep=0x224c wordp=0x72e3
// ================================================

void FILE_st_() // FILE<
{
  Func6("FILE-ST");
  _2_at_(); // 2@
  SWAP(); // SWAP
  OVER(); // OVER
  _dash_(); // -
  Push(Pop() + 1); //  1+
  MEM_st_DSK(); // MEM<DSK
}


// ================================================
// 0x72f3: WORD 'W72F5' codep=0x224c wordp=0x72f5 params=0 returns=0
// ================================================

void W72F5() // W72F5
{
  W6F27(); // W6F27
  if (Pop() == 0) return;
  MOUNTB(); // MOUNTB
}


// ================================================
// 0x72ff: WORD 'W7301' codep=0x224c wordp=0x7301
// ================================================

void W7301() // W7301
{
  W72F5(); // W72F5
  W723F(); // W723F
  Push(4);
  PICK(); // PICK
  Push(pp_W5603); // W5603
  _st__ex__gt_(); // <!>
  W7137(); // W7137
  BLOCK_1(); // BLOCK_1
  Push(Pop() + Pop()); // +
  Push(Read16(regsp)); // DUP
  Push(pp_W5607); // W5607
  _st__ex__gt_(); // <!>
  SWAP(); // SWAP
  Push(pp_W55FF); // W55FF
  _st__ex__gt_(); // <!>
}


// ================================================
// 0x7321: WORD 'W7323' codep=0x224c wordp=0x7323 params=2 returns=3
// ================================================

void W7323() // W7323
{
  OVER(); // OVER
  Push(Pop()==Read16(pp_W55FF)?1:0); //  W55FF @ =
  OVER(); // OVER
  Push(Pop()==Read16(pp_W5603)?1:0); //  W5603 @ =
  Push(Pop() & Pop()); // AND
  Push(!Pop()); //  NOT
}


// ================================================
// 0x7339: WORD '@RECORD' codep=0x224c wordp=0x7345 params=2 returns=1
// ================================================

void GetRECORD() // @RECORD
{
  W7323(); // W7323
  if (Pop() != 0)
  {
    OVER(); // OVER
    Func6("FILE-RL");
    Push(Read16(Pop())&0xFF); //  C@
    Push(3);
    PICK(); // PICK
    Func6("FILE-ST");
    Push(Read16(Pop())); //  @
    W7301(); // W7301
    return;
  }
  Pop(); Pop(); // 2DROP
  Push(Read16(pp_W5607)); // W5607 @
}


// ================================================
// 0x7367: WORD 'AFIELD' codep=0x224c wordp=0x7372
// ================================================

void AFIELD() // AFIELD
{
  unsigned short int a, b;
  Exec("CREATE"); // call of word 0x1cbb '(CREATE)'
  ROT(); // ROT
  Push(Read16(regsp)); // DUP
  C_co_(); // C,
  a = Pop(); // >R
  SWAP(); // SWAP
  C_co_(); // C,
  C_co_(); // C,
  Push(a); // I
  Func6("FILE-RL");
  Push(Read16(Pop())&0xFF); //  C@
  C_co_(); // C,
  Push(a); // R>
  Func6("FILE-ST");
  Push(Read16(Pop())); //  @
  _co_(); // ,
  CODE(); // (;CODE) inlined assembler code
// 0x7394: call   1649
  b = Pop(); // >R
  Push(!((Read16(b)&0xFF)==Read16(pp_FILE_n_)?1:0)); // I C@ FILE# @ = NOT
  if (Pop() != 0)
  {
    _gt_0FONT_1(); // >0FONT_1
    PRINT("AFIELD err", 10); // (.")
    Exec("CR"); // call of word 0x26ee '(CR)'
    Push(b); // R>
    UNRAVEL(); // UNRAVEL
    return;
  }
  Push(Read16(pp_FILE_n_)); // FILE# @
  Push(Read16(pp_RECORD_n_)); // RECORD# @
  W7323(); // W7323
  if (Pop() != 0)
  {
    Push(Read16(b + 3)&0xFF); // I 3 + C@
    Push(Read16(b + 4)); // I 4 + @
    W7301(); // W7301
  } else
  {
    Pop(); Pop(); // 2DROP
    Push(Read16(pp_W5607)); // W5607 @
  }
  Push(Pop() + (Read16(b + 1)&0xFF)); //  R> 1+ C@ +
}


// ================================================
// 0x73f6: WORD 'W73F8' codep=0x73f8 wordp=0x73f8
// ================================================
// 0x73f8: pop    bx
// 0x73f9: xor    ax,ax
// 0x73fb: mov    al,[bx+01]
// 0x73fe: add    ax,65E1
// 0x7401: push   ax
// 0x7402: lodsw
// 0x7403: mov    bx,ax
// 0x7405: jmp    word ptr [bx]

// ================================================
// 0x7407: WORD 'IFIELD' codep=0x224c wordp=0x7412
// ================================================

void IFIELD() // IFIELD
{
  unsigned short int a;
  Exec("CREATE"); // call of word 0x1cbb '(CREATE)'
  ROT(); // ROT
  C_co_(); // C,
  SWAP(); // SWAP
  C_co_(); // C,
  C_co_(); // C,
  CODE(); // (;CODE) inlined assembler code
// 0x7420: call   1649
  Push(Read16(regsp)); // DUP
  a = Pop(); // >R
  Push(Read16(Pop())&0xFF); //  C@
  if (Pop() != 0)
  {
    Push(!(Read16(pp_FILE_n_)==(Read16(a)&0xFF)?1:0)); // FILE# @ I C@ = NOT
    if (Pop() != 0)
    {
      _gt_0FONT_1(); // >0FONT_1
      PRINT("IFIELD err", 10); // (.")
      Exec("CR"); // call of word 0x26ee '(CR)'
      Push(a); // R>
      UNRAVEL(); // UNRAVEL
    }
  }
  Push(unknown); // R>
  W73F8(); // W73F8
}


// ================================================
// 0x7458: WORD 'INST-SI' codep=0x7420 wordp=0x7464
// ================================================
IFieldType INST_dash_SI = {DIRECTORYIDX, 0x00, 0x03};

// ================================================
// 0x7467: WORD 'INST-PR' codep=0x7420 wordp=0x7473
// ================================================
IFieldType INST_dash_PR = {DIRECTORYIDX, 0x03, 0x03};

// ================================================
// 0x7476: WORD 'INST-OF' codep=0x7420 wordp=0x7482
// ================================================
IFieldType INST_dash_OF = {DIRECTORYIDX, 0x06, 0x03};

// ================================================
// 0x7485: WORD 'INST-CL' codep=0x7420 wordp=0x7491
// ================================================
IFieldType INST_dash_CL = {DIRECTORYIDX, 0x09, 0x01};

// ================================================
// 0x7494: WORD 'INST-SP' codep=0x7420 wordp=0x74a0
// ================================================
IFieldType INST_dash_SP = {DIRECTORYIDX, 0x0a, 0x01};

// ================================================
// 0x74a3: WORD 'INST-QT' codep=0x7420 wordp=0x74af
// ================================================
IFieldType INST_dash_QT = {DIRECTORYIDX, 0x0b, 0x02};

// ================================================
// 0x74b2: WORD 'INST-X' codep=0x7420 wordp=0x74bd
// ================================================
IFieldType INST_dash_X = {DIRECTORYIDX, 0x0d, 0x02};

// ================================================
// 0x74c0: WORD 'INST-Y' codep=0x7420 wordp=0x74cb
// ================================================
IFieldType INST_dash_Y = {DIRECTORYIDX, 0x0f, 0x02};

// ================================================
// 0x74ce: WORD 'W74D0' codep=0x224c wordp=0x74d0 params=2 returns=0
// ================================================

void W74D0() // W74D0
{
  Push(0x65e1+INST_dash_SI.offset); // INST-SI<IFIELD>
  _1_dot_5_ex__2(); // 1.5!_2
}


// ================================================
// 0x74d6: WORD 'W74D8' codep=0x224c wordp=0x74d8 params=2 returns=0
// ================================================

void W74D8() // W74D8
{
  Push(0x65e1+INST_dash_PR.offset); // INST-PR<IFIELD>
  _1_dot_5_ex__2(); // 1.5!_2
}


// ================================================
// 0x74de: WORD 'W74E0' codep=0x224c wordp=0x74e0 params=2 returns=0
// ================================================

void W74E0() // W74E0
{
  Push(0x65e1+INST_dash_OF.offset); // INST-OF<IFIELD>
  _1_dot_5_ex__2(); // 1.5!_2
}


// ================================================
// 0x74e6: WORD 'W74E8' codep=0x224c wordp=0x74e8 params=1 returns=0
// ================================================

void W74E8() // W74E8
{
  Push(0x65e1+INST_dash_CL.offset); // INST-CL<IFIELD>
  C_ex__2(); // C!_2
}


// ================================================
// 0x74ee: WORD '!INST-S' codep=0x224c wordp=0x74fa params=1 returns=0
// ================================================

void StoreINST_dash_S() // !INST-S
{
  Push(0x65e1+INST_dash_SP.offset); // INST-SP<IFIELD>
  C_ex__2(); // C!_2
}


// ================================================
// 0x7500: WORD 'W7502' codep=0x224c wordp=0x7502 params=0 returns=2
// ================================================

void W7502() // W7502
{
  Push(0x65e1+INST_dash_SI.offset); // INST-SI<IFIELD>
  _1_dot_5_at_(); // 1.5@
}


// ================================================
// 0x7508: WORD 'W750A' codep=0x224c wordp=0x750a params=0 returns=2
// ================================================

void W750A() // W750A
{
  Push(0x65e1+INST_dash_PR.offset); // INST-PR<IFIELD>
  _1_dot_5_at_(); // 1.5@
}


// ================================================
// 0x7510: WORD 'W7512' codep=0x224c wordp=0x7512 params=0 returns=2
// ================================================

void W7512() // W7512
{
  Push(0x65e1+INST_dash_OF.offset); // INST-OF<IFIELD>
  _1_dot_5_at_(); // 1.5@
}


// ================================================
// 0x7518: WORD '@INST-C' codep=0x224c wordp=0x7524 params=0 returns=1
// ================================================

void GetINST_dash_C() // @INST-C
{
  Push(Read16(0x65e1+INST_dash_CL.offset)&0xFF); // INST-CL<IFIELD> C@
}


// ================================================
// 0x752a: WORD '@INST-S' codep=0x224c wordp=0x7536 params=0 returns=1
// ================================================

void GetINST_dash_S() // @INST-S
{
  Push(Read16(0x65e1+INST_dash_SP.offset)&0xFF); // INST-SP<IFIELD> C@
}


// ================================================
// 0x753c: WORD '=SPECIE' codep=0x224c wordp=0x7548 params=1 returns=1
// ================================================

void _eq_SPECIE() // =SPECIE
{
  GetINST_dash_S(); // @INST-S
  Push((Pop()==Pop())?1:0); // =
}


// ================================================
// 0x754e: WORD '>C' codep=0x7555 wordp=0x7555 params=2 returns=0
// ================================================
// 0x7555: mov    bx,[5634] // CXSP
// 0x7559: pop    ax
// 0x755a: mov    [bx+02],al
// 0x755d: pop    word ptr [bx]
// 0x755f: sub    word ptr [5634],03 // CXSP
// 0x7564: lodsw
// 0x7565: mov    bx,ax
// 0x7567: jmp    word ptr [bx]

// ================================================
// 0x7569: WORD 'C>' codep=0x7570 wordp=0x7570 params=0 returns=2
// ================================================
// 0x7570: add    word ptr [5634],03 // CXSP
// 0x7575: mov    bx,[5634] // CXSP
// 0x7579: push   word ptr [bx]
// 0x757b: xor    ax,ax
// 0x757d: mov    al,[bx+02]
// 0x7580: push   ax
// 0x7581: lodsw
// 0x7582: mov    bx,ax
// 0x7584: jmp    word ptr [bx]

// ================================================
// 0x7586: WORD 'CI' codep=0x758d wordp=0x758d params=0 returns=2
// ================================================
// 0x758d: mov    bx,[5634] // CXSP
// 0x7591: push   word ptr [bx+03]
// 0x7594: xor    ax,ax
// 0x7596: mov    al,[bx+05]
// 0x7599: push   ax
// 0x759a: lodsw
// 0x759b: mov    bx,ax
// 0x759d: jmp    word ptr [bx]

// ================================================
// 0x759f: WORD 'CDROP' codep=0x224c wordp=0x75a9 params=0 returns=0
// ================================================

void CDROP() // CDROP
{
  C_gt_(); // C>
  Pop(); Pop(); // 2DROP
}


// ================================================
// 0x75af: WORD 'CI'' codep=0x224c wordp=0x75b7 params=0 returns=2
// ================================================

void CI_i_() // CI'
{
  C_gt_(); // C>
  CI(); // CI
  _2SWAP(); // 2SWAP
  _gt_C(); // >C
}


// ================================================
// 0x75c1: WORD 'CJ' codep=0x224c wordp=0x75c8 params=0 returns=2
// ================================================

void CJ() // CJ
{
  C_gt_(); // C>
  CI_i_(); // CI'
  _2SWAP(); // 2SWAP
  _gt_C(); // >C
}


// ================================================
// 0x75d2: WORD 'COVER' codep=0x224c wordp=0x75dc params=0 returns=0
// ================================================

void COVER() // COVER
{
  CI_i_(); // CI'
  _gt_C(); // >C
}


// ================================================
// 0x75e2: WORD 'CDEPTH' codep=0x75ed wordp=0x75ed params=0 returns=1
// ================================================
// 0x75ed: mov    ax,658A
// 0x75f0: sub    ax,[5634] // CXSP
// 0x75f4: mov    cx,0003
// 0x75f7: div    cl
// 0x75f9: push   ax
// 0x75fa: lodsw
// 0x75fb: mov    bx,ax
// 0x75fd: jmp    word ptr [bx]

// ================================================
// 0x75ff: WORD '?NULL' codep=0x224c wordp=0x7609 params=0 returns=1
// ================================================

void IsNULL() // ?NULL
{
  CI(); // CI
  D0_eq_(); // D0=
}


// ================================================
// 0x760f: WORD '?-NULL' codep=0x224c wordp=0x761a params=0 returns=1
// ================================================

void Is_dash_NULL() // ?-NULL
{
  IsNULL(); // ?NULL
  Push(!Pop()); //  NOT
}


// ================================================
// 0x7620: WORD '?CHILD' codep=0x224c wordp=0x762b params=0 returns=1
// ================================================

void IsCHILD() // ?CHILD
{
  W7512(); // W7512
  D0_eq_(); // D0=
  Push(!Pop()); //  NOT
}


// ================================================
// 0x7633: WORD 'W7635' codep=0x224c wordp=0x7635 params=0 returns=1
// ================================================

void W7635() // W7635
{
  CI(); // CI
  W7502(); // W7502
  D_eq_(); // D=
}


// ================================================
// 0x763d: WORD '!IADDR' codep=0x224c wordp=0x7648 params=2 returns=0
// ================================================

void StoreIADDR() // !IADDR
{
  Push(pp_W639C); // W639C
  _st_1_dot_5_ex__gt_(); // <1.5!>
}


// ================================================
// 0x764e: WORD 'W7650' codep=0x7650 wordp=0x7650 params=0 returns=1
// ================================================
// 0x7650: mov    ax,[56F4] // W56F4
// 0x7654: push   ax
// 0x7655: lodsw
// 0x7656: mov    bx,ax
// 0x7658: jmp    word ptr [bx]

// ================================================
// 0x765a: WORD 'W765C' codep=0x224c wordp=0x765c params=0 returns=0
// ================================================

void W765C() // W765C
{
  GetDS(); // @DS
  Push(pp_IBFR); // IBFR
  Push(Read16(pp_W5676)); // W5676 @
  Push(Read16(pp_W56EC)); // W56EC @
  Push(Read16(pp_W56F8)); // W56F8 @
  LCMOVE(); // LCMOVE
}


// ================================================
// 0x7670: WORD 'W7672' codep=0x224c wordp=0x7672 params=0 returns=0
// ================================================

void W7672() // W7672
{
  Push(Read16(pp_W5676)); // W5676 @
  Push(Read16(pp_W56EC)); // W56EC @
  GetDS(); // @DS
  Push(pp_IBFR); // IBFR
  Push(Read16(pp_W56F8)); // W56F8 @
  LCMOVE(); // LCMOVE
}


// ================================================
// 0x7686: WORD 'W7688' codep=0x224c wordp=0x7688 params=2 returns=2
// ================================================

void W7688() // W7688
{
  W72F5(); // W72F5
  VA_gt_BUF(); // VA>BUF
}


// ================================================
// 0x768e: WORD 'PRIORIT' codep=0x769a wordp=0x769a
// ================================================
// 0x769a: pop    ax
// 0x769b: or     ax,ax
// 0x769d: jz     76C6
// 0x769f: mov    bx,ax
// 0x76a1: sub    bx,02
// 0x76a4: push   es
// 0x76a5: push   word ptr [566A] // W566A
// 0x76a9: pop    es
// 0x76aa: call   2F36
// 0x76ad: push   word ptr [5672] // W5672
// 0x76b1: pop    es
// 0x76b2: call   2F36
// 0x76b5: shr    bx,1
// 0x76b7: shr    ax,1
// 0x76b9: push   word ptr [566E] // W566E
// 0x76bd: pop    es
// 0x76be: call   4AB8
// 0x76c1: pop    es
// 0x76c2: shl    bx,1
// 0x76c4: mov    ax,bx
// 0x76c6: push   ax
// 0x76c7: lodsw
// 0x76c8: mov    bx,ax
// 0x76ca: jmp    word ptr [bx]

// ================================================
// 0x76cc: WORD 'W76CE' codep=0x224c wordp=0x76ce
// ================================================

void W76CE() // W76CE
{
  Push(Read16(regsp)); // DUP
  Push(Read16(Pop() + 9)&0xFF); //  9 + C@
  Push(Read16(regsp)); // DUP
  Push(Pop()==0x0030?1:0); //  0x0030 =
  if (Pop() != 0)
  {
    Pop(); // DROP
    Push((Read16(Pop() + 0x000b)&0xFF) + 1); //  0x000b + C@ 1+
  } else
  {
    SWAP(); // SWAP
    Pop(); // DROP
    Func6("FILE-SL");
    Push(Read16(Pop())&0xFF); //  C@
  }
  Push(Pop() + Read16(cc_IHEADLE)); //  IHEADLE +
}


// ================================================
// 0x7700: WORD 'W7702' codep=0x224c wordp=0x7702 params=1 returns=0
// ================================================

void W7702() // W7702
{
  Push(pp_W56F4); // W56F4
  _st__ex__gt_(); // <!>
  Push(Read16(pp_W5676)); // W5676 @
  Push(Read16(pp_W5672)); // W5672 @
  W7650(); // W7650
  L_at_(); // L@
  Push(Read16(regsp)); // DUP
  Push(pp_W56EC); // W56EC
  _st__ex__gt_(); // <!>
  L_at_(); // L@
  Push(pp_W56F8); // W56F8
  _st__ex__gt_(); // <!>
  Push(Read16(pp_W566A)); // W566A @
  W7650(); // W7650
  L_at_(); // L@
  Push(Read16(pp_W566E)); // W566E @
  W7650(); // W7650
  Push(Pop() >> 1); //  2/
  LC_at_(); // LC@
  StoreIADDR(); // !IADDR
}


// ================================================
// 0x7734: WORD 'W7736' codep=0x224c wordp=0x7736 params=2 returns=1
// ================================================

void W7736() // W7736
{
  unsigned short int a;
  Push(Read16(pp_W56E0)); // W56E0 @
  if (Pop() != 0)
  {
    SWAP(); // SWAP
    a = Pop(); // >R
    Push(Read16(pp_W566A)); // W566A @
    Push(0);
    Push(Read16(pp_W56E0)); // W56E0 @
    Push(a); // I
    LWSCAN(); // LWSCAN
    if (Pop() != 0)
    {
      Push(Read16(regsp)); // DUP
      Push(Pop() >> 1); //  2/
      Push(Read16(pp_W566E)); // W566E @
      SWAP(); // SWAP
      LC_at_(); // LC@
      ROT(); // ROT
      Push((Pop()==Pop())?1:0); // =
      Push(!Pop()); //  NOT
      if (Pop() != 0)
      {
        Push(Read16(pp_W566A)); // W566A @
        SWAP(); // SWAP
        Push(Pop() + 2); //  2+
        Push(Read16(pp_W56E0)); // W56E0 @
        OVER(); // OVER
        Push(Pop() >> 1); //  2/
        _dash_(); // -
        Push(a); // I
        LWSCAN(); // LWSCAN
      } else
      {
        Push(1);
      }
    } else
    {
      Pop(); // DROP
      Push(0);
    }
    Push(a); // R>
    Pop(); // DROP
    return;
  }
  Pop(); Pop(); // 2DROP
  Push(0);
}


// ================================================
// 0x779c: WORD 'W779E' codep=0x224c wordp=0x779e params=2 returns=1
// ================================================

void W779E() // W779E
{
  Push(Pop() + 2); //  2+
  LC_at_(); // LC@
}


// ================================================
// 0x77a4: WORD 'W77A6' codep=0x224c wordp=0x77a6 params=1 returns=2
// ================================================

void W77A6() // W77A6
{
  unsigned short int a;
  a = Pop(); // >R
  Push(Read16(pp_W5672)); // W5672 @
  Push(0);
  Push(Read16(pp_W56E0)); // W56E0 @
  Push(a); // R>
  LWSCAN(); // LWSCAN
}


// ================================================
// 0x77b8: WORD 'W77BA' codep=0x224c wordp=0x77ba params=0 returns=0
// ================================================

void W77BA() // W77BA
{
  Push(Read16(pp_IBFR + 2)&0xFF); // IBFR 2+ C@
  if (Pop() == 0) return;
  W765C(); // W765C
  Push(0);
  Push(pp_IBFR + 2); // IBFR 2+
  _st_C_ex__gt_(); // <C!>
  Push2Words("NULL");
  StoreIADDR(); // !IADDR
}


// ================================================
// 0x77d4: WORD 'W77D6' codep=0x224c wordp=0x77d6 params=0 returns=0
// ================================================

void W77D6() // W77D6
{
  Push(Read16(pp_W5676)); // W5676 @
  Push(Read16(pp_W56EC)); // W56EC @
  W779E(); // W779E
  if (Pop() == 0) return;
  Push(Read16(pp_W5676)); // W5676 @
  Push(Read16(pp_W56EC) + 3); // W56EC @ 3 +
  GetDS(); // @DS
  Push(pp_W639C); // W639C
  _1_dot_5_at_(); // 1.5@
  W7688(); // W7688
  IsUPDATE(); // ?UPDATE
  Push(Read16(pp_W56F8) - 3); // W56F8 @ 3 -
  LCMOVE(); // LCMOVE
  Push(0);
  Push(Read16(pp_W5676)); // W5676 @
  Push(Read16(pp_W56EC) + 2); // W56EC @ 2+
  LC_ex_(); // LC!
}


// ================================================
// 0x7814: WORD 'W7816' codep=0x224c wordp=0x7816 params=0 returns=1
// ================================================

void W7816() // W7816
{
  Push((Read16(pp_W56E0) - 1) * 2); // W56E0 @ 1- 2*
  W7702(); // W7702
  W77D6(); // W77D6
  Push(-1);
  Push(pp_W56E0); // W56E0
  _st__plus__ex__gt_(); // <+!>
  Push(-Read16(pp_W56F8)); // W56F8 @ NEGATE
  Push(pp_W56E8); // W56E8
  _st__plus__ex__gt_(); // <+!>
  Push(0);
  Push(Read16(pp_W566A)); // W566A @
  W7650(); // W7650
  L_ex_(); // L!
  Push(0x270f);
  Push(Read16(pp_W5672)); // W5672 @
  W7650(); // W7650
  L_ex_(); // L!
  Push(Read16(pp_W56EC)); // W56EC @
  Push(pp_W56F0); // W56F0
  _st__ex__gt_(); // <!>
  while(1)
  {
    Push(Read16(pp_W56EC) + Read16(pp_W56F8)); // W56EC @ W56F8 @ +
    W77A6(); // W77A6
    if (Pop() == 0) return;
    W7702(); // W7702
    Push(Read16(pp_W5676)); // W5676 @
    Push(Read16(pp_W56EC)); // W56EC @
    OVER(); // OVER
    Push(Read16(pp_W56F0)); // W56F0 @
    Push(Read16(pp_W56F8)); // W56F8 @
    LCMOVE(); // LCMOVE
    Push(Read16(pp_W56F0)); // W56F0 @
    Push(Read16(pp_W5672)); // W5672 @
    W7650(); // W7650
    L_ex_(); // L!
    Push(Read16(pp_W56F8)); // W56F8 @
    Push(pp_W56F0); // W56F0
    _st__plus__ex__gt_(); // <+!>
  }
}


// ================================================
// 0x7890: WORD 'W7892' codep=0x224c wordp=0x7892 params=0 returns=0
// ================================================

void W7892() // W7892
{
  while(1)
  {
    Push(Read16(pp_W56E4) - Read16(pp_W56E8)); // W56E4 @ W56E8 @ -
    Push(0x0111);
    _st_(); // <
    if (Pop() == 0) return;
    W7816(); // W7816
  }
}


// ================================================
// 0x78ae: WORD '1BTN' codep=0x7394 wordp=0x78b7
// ================================================
LoadDataType _1BTN = {BUTTONSIDX, 0x00, 0x0c, 0x49, 0x6e57};

// ================================================
// 0x78bd: WORD 'AFIELD:' codep=0x224c wordp=0x78c9
// ================================================

void AFIELD_c_() // AFIELD:
{
  unsigned short int a;
  HEAD_c_(); // HEAD:
  Push(0x7394);
  _co_(); // ,
  ROT(); // ROT
  Push(Read16(regsp)); // DUP
  C_co_(); // C,
  a = Pop(); // >R
  SWAP(); // SWAP
  C_co_(); // C,
  C_co_(); // C,
  Push(a); // I
  Func6("FILE-RL");
  Push(Read16(Pop())&0xFF); //  C@
  C_co_(); // C,
  Push(a); // R>
  Func6("FILE-ST");
  Push(Read16(Pop())); //  @
  _co_(); // ,
}


// ================================================
// 0x78f1: WORD 'IFIELD:' codep=0x224c wordp=0x78fd
// ================================================

void IFIELD_c_() // IFIELD:
{
  HEAD_c_(); // HEAD:
  Push(0x7420);
  _co_(); // ,
  ROT(); // ROT
  C_co_(); // C,
  SWAP(); // SWAP
  C_co_(); // C,
  C_co_(); // C,
}


// ================================================
// 0x7911: WORD '#BTN' codep=0x7394 wordp=0x791a
// ================================================
LoadDataType _n_BTN = {BUTTONSIDX, 0x48, 0x01, 0x49, 0x6e57};

// ================================================
// 0x7920: WORD 'W7922' codep=0x224c wordp=0x7922
// ================================================

void W7922() // W7922
{
  W7892(); // W7892
  _2DUP(); // 2DUP
  StoreIADDR(); // !IADDR
  W7688(); // W7688
  Push(Read16(regsp)); // DUP
  W76CE(); // W76CE
  Push(Read16(regsp)); // DUP
  Push(Pop() + 3); //  3 +
  Push(Read16(regsp)); // DUP
  Push(pp_IBFR); // IBFR
  Store_2(); // !_2
  Push(pp_W56F8); // W56F8
  _st__ex__gt_(); // <!>
  Push(pp_IBFR + 3); // IBFR 3 +
  SWAP(); // SWAP
  CMOVE_2(); // CMOVE_2
  Push(0);
  Push(pp_IBFR + 2); // IBFR 2+
  _st_C_ex__gt_(); // <C!>
  Push(Read16(pp_W56E0) * 2); // W56E0 @ 2*
  Push(pp_W56F4); // W56F4
  _st__ex__gt_(); // <!>
  Push(1);
  Push(pp_W56E0); // W56E0
  _st__plus__ex__gt_(); // <+!>
  Push(pp_W639C); // W639C
  _1_dot_5_at_(); // 1.5@
  Push(Read16(pp_W566E)); // W566E @
  W7650(); // W7650
  Push(Pop() >> 1); //  2/
  LC_ex_(); // LC!
  Push(Read16(pp_W566A)); // W566A @
  W7650(); // W7650
  L_ex_(); // L!
  Push(Read16(pp_W56E8)); // W56E8 @
  Push(pp_W56EC); // W56EC
  _st__ex__gt_(); // <!>
  Push(Read16(pp_W56F8)); // W56F8 @
  Push(pp_W56E8); // W56E8
  _st__plus__ex__gt_(); // <+!>
  Push(Read16(pp_W56EC)); // W56EC @
  Push(Read16(pp_W5672)); // W5672 @
  W7650(); // W7650
  L_ex_(); // L!
  W765C(); // W765C
}


// ================================================
// 0x7996: WORD 'SET-CUR' codep=0x224c wordp=0x79a2 params=0 returns=0
// ================================================

void SET_dash_CUR() // SET-CUR
{
  CDEPTH(); // CDEPTH
  _0_gt_(); // 0>
  if (Pop() == 0) return;
  W77BA(); // W77BA
  Is_dash_NULL(); // ?-NULL
  if (Pop() == 0) return;
  CI(); // CI
  W7736(); // W7736
  if (Pop() != 0)
  {
    PRIORIT(); // PRIORIT
    W7702(); // W7702
    W7672(); // W7672
  } else
  {
    CI(); // CI
    W7922(); // W7922
  }
  GetINST_dash_C(); // @INST-C
  Push(pp_FILE_n_); // FILE#
  _st__ex__gt_(); // <!>
  GetINST_dash_S(); // @INST-S
  Push(pp_RECORD_n_); // RECORD#
  _st__ex__gt_(); // <!>
}


// ================================================
// 0x79d6: WORD 'ICLOSE' codep=0x224c wordp=0x79e1 params=0 returns=0
// ================================================

void ICLOSE() // ICLOSE
{
  W77BA(); // W77BA
  CDROP(); // CDROP
  COVER(); // COVER
  Is_dash_NULL(); // ?-NULL
  if (Pop() != 0)
  {
    SET_dash_CUR(); // SET-CUR
    W7512(); // W7512
  } else
  {
    Push2Words("NULL");
  }
  Push(pp_W63A2); // W63A2
  _st_1_dot_5_ex__gt_(); // <1.5!>
  CDROP(); // CDROP
  SET_dash_CUR(); // SET-CUR
}


// ================================================
// 0x7a01: WORD '*CLOSE' codep=0x224c wordp=0x7a0c params=1 returns=0
// ================================================

void _star_CLOSE() // *CLOSE
{
  unsigned short int i, imax;

  i = 0;
  imax = Pop();
  do // (DO)
  {
    ICLOSE(); // ICLOSE
    i++;
  } while(i<imax); // (LOOP)

}


// ================================================
// 0x7a18: WORD '>C+S' codep=0x224c wordp=0x7a21 params=2 returns=0
// ================================================

void _gt_C_plus_S() // >C+S
{
  _gt_C(); // >C
  SET_dash_CUR(); // SET-CUR
}


// ================================================
// 0x7a27: WORD '@>C+S' codep=0x224c wordp=0x7a31 params=1 returns=0
// ================================================

void Get_gt_C_plus_S() // @>C+S
{
  _1_dot_5_at_(); // 1.5@
  _gt_C_plus_S(); // >C+S
}


// ================================================
// 0x7a37: WORD 'IOPEN' codep=0x224c wordp=0x7a41 params=0 returns=0
// ================================================

void IOPEN() // IOPEN
{
  W7512(); // W7512
  _2DUP(); // 2DUP
  Push(pp_W63A2); // W63A2
  _st_1_dot_5_ex__gt_(); // <1.5!>
  _gt_C_plus_S(); // >C+S
}


// ================================================
// 0x7a4d: WORD 'CCLR' codep=0x224c wordp=0x7a56 params=0 returns=0
// ================================================

void CCLR() // CCLR
{
  W77BA(); // W77BA
  Push(Read16(cc_W50FA)); // W50FA
  Push(pp_CXSP); // CXSP
  _st__ex__gt_(); // <!>
  Push(pp_W651B); // W651B
  Push(0x0078);
  Push(0);
  FILL_2(); // FILL_2
}


// ================================================
// 0x7a6a: WORD 'W7A6C' codep=0x224c wordp=0x7a6c params=0 returns=2
// ================================================

void W7A6C() // W7A6C
{
  Push(pp_W63A2); // W63A2
  _1_dot_5_at_(); // 1.5@
}


// ================================================
// 0x7a72: WORD '?LAST' codep=0x224c wordp=0x7a7c params=0 returns=1
// ================================================

void IsLAST() // ?LAST
{
  W7502(); // W7502
  W7A6C(); // W7A6C
  D_eq_(); // D=
}


// ================================================
// 0x7a84: WORD '?FIRST' codep=0x224c wordp=0x7a8f params=0 returns=1
// ================================================

void IsFIRST() // ?FIRST
{
  CI(); // CI
  W7A6C(); // W7A6C
  D_eq_(); // D=
}


// ================================================
// 0x7a97: WORD '>C+' codep=0x224c wordp=0x7a9f params=2 returns=0
// ================================================

void _gt_C_plus_() // >C+
{
  SET_dash_CUR(); // SET-CUR
  IOPEN(); // IOPEN
  CDROP(); // CDROP
  _gt_C(); // >C
}


// ================================================
// 0x7aa9: WORD 'INEXT' codep=0x224c wordp=0x7ab3 params=0 returns=0
// ================================================

void INEXT() // INEXT
{
  Is_dash_NULL(); // ?-NULL
  if (Pop() == 0) return;
  W7502(); // W7502
  CDROP(); // CDROP
  _gt_C_plus_S(); // >C+S
}


// ================================================
// 0x7ac1: WORD 'IPREV' codep=0x224c wordp=0x7acb params=0 returns=0
// ================================================

void IPREV() // IPREV
{
  Is_dash_NULL(); // ?-NULL
  if (Pop() == 0) return;
  W750A(); // W750A
  CDROP(); // CDROP
  _gt_C_plus_S(); // >C+S
}


// ================================================
// 0x7ad9: WORD 'IFIRST' codep=0x224c wordp=0x7ae4 params=0 returns=0
// ================================================

void IFIRST() // IFIRST
{
  CDROP(); // CDROP
  W7A6C(); // W7A6C
  _gt_C_plus_S(); // >C+S
}


// ================================================
// 0x7aec: WORD 'ILAST' codep=0x224c wordp=0x7af6 params=0 returns=0
// ================================================

void ILAST() // ILAST
{
  IFIRST(); // IFIRST
  IPREV(); // IPREV
}


// ================================================
// 0x7afc: WORD 'VCLR' codep=0x224c wordp=0x7b05 params=0 returns=0
// ================================================

void VCLR() // VCLR
{
  Push(Read16(cc_W50FE)); // W50FE
  Push(pp_W5638); // W5638
  _st__ex__gt_(); // <!>
}


// ================================================
// 0x7b0d: WORD '>V' codep=0x7b14 wordp=0x7b14 params=1 returns=0
// ================================================
// 0x7b14: mov    bx,[5638] // W5638
// 0x7b18: pop    word ptr [bx]
// 0x7b1a: sub    word ptr [5638],02 // W5638
// 0x7b1f: lodsw
// 0x7b20: mov    bx,ax
// 0x7b22: jmp    word ptr [bx]

// ================================================
// 0x7b24: WORD 'V>' codep=0x7b2b wordp=0x7b2b params=0 returns=1
// ================================================
// 0x7b2b: add    word ptr [5638],02 // W5638
// 0x7b30: mov    bx,[5638] // W5638
// 0x7b34: push   word ptr [bx]
// 0x7b36: lodsw
// 0x7b37: mov    bx,ax
// 0x7b39: jmp    word ptr [bx]

// ================================================
// 0x7b3b: WORD 'VI' codep=0x7b42 wordp=0x7b42 params=0 returns=1
// ================================================
// 0x7b42: mov    bx,[5638] // W5638
// 0x7b46: push   word ptr [bx+02]
// 0x7b49: lodsw
// 0x7b4a: mov    bx,ax
// 0x7b4c: jmp    word ptr [bx]

// ================================================
// 0x7b4e: WORD 'W7B50' codep=0x224c wordp=0x7b50 params=0 returns=0
// ================================================
// orphan

void W7B50() // W7B50
{
  unsigned short int i, imax;
  W77BA(); // W77BA
  Push(Read16(pp_W56E0)); // W56E0 @
  if (Read16(regsp) != 0) Push(Read16(regsp)); // ?DUP
  if (Pop() == 0) return;

  i = 0;
  imax = Pop();
  do // (DO)
  {
    Push(i * 2); // I 2*
    W7702(); // W7702
    W77D6(); // W77D6
    i++;
  } while(i<imax); // (LOOP)

  SET_dash_CUR(); // SET-CUR
}


// ================================================
// 0x7b70: WORD 'SAVE-BU' codep=0x224c wordp=0x7b7c
// ================================================

void SAVE_dash_BU() // SAVE-BU
{
  SAVE_dash_BUFFERS(); // SAVE-BUFFERS
  INIT(); // INIT
}


// ================================================
// 0x7b82: WORD 'FLUSH_2' codep=0x224c wordp=0x7b8c
// ================================================

void FLUSH_2() // FLUSH_2
{
  SAVE_dash_BU(); // SAVE-BU
  EMPTY_dash_BUFFERS(); // EMPTY-BUFFERS
}


// ================================================
// 0x7b92: WORD 'MT' codep=0x224c wordp=0x7b99 params=0 returns=0
// ================================================

void MT() // MT
{
  EMPTY_dash_BUFFERS(); // EMPTY-BUFFERS
  EMPTY(); // EMPTY
}


// ================================================
// 0x7b9f: WORD 'IINSERT' codep=0x224c wordp=0x7bab params=4 returns=0
// ================================================

void IINSERT() // IINSERT
{
  _gt_C_plus_S(); // >C+S
  IsCHILD(); // ?CHILD
  if (Pop() != 0)
  {
    W7512(); // W7512
    _2SWAP(); // 2SWAP
    W74E0(); // W74E0
    IOPEN(); // IOPEN
    W74D0(); // W74D0
    CI(); // CI
    INEXT(); // INEXT
    W750A(); // W750A
    _2SWAP(); // 2SWAP
    W74D8(); // W74D8
    IPREV(); // IPREV
    W74D8(); // W74D8
    CI(); // CI
    IPREV(); // IPREV
    W74D0(); // W74D0
  } else
  {
    W74E0(); // W74E0
    IOPEN(); // IOPEN
    CI(); // CI
    W74D0(); // W74D0
    CI(); // CI
    W74D8(); // W74D8
  }
  CDROP(); // CDROP
  ICLOSE(); // ICLOSE
}


// ================================================
// 0x7be7: WORD '<INSERT' codep=0x224c wordp=0x7bf3 params=4 returns=0
// ================================================

void _st_INSERT() // <INSERT
{
  _gt_C_plus_S(); // >C+S
  W750A(); // W750A
  _2SWAP(); // 2SWAP
  W74D8(); // W74D8
  CI(); // CI
  IPREV(); // IPREV
  W74D0(); // W74D0
  W74D8(); // W74D8
  CI(); // CI
  IPREV(); // IPREV
  W74D0(); // W74D0
  ICLOSE(); // ICLOSE
}


// ================================================
// 0x7c0d: WORD '>INSERT' codep=0x224c wordp=0x7c19 params=4 returns=0
// ================================================

void _gt_INSERT() // >INSERT
{
  _gt_C_plus_S(); // >C+S
  W7502(); // W7502
  ICLOSE(); // ICLOSE
  _st_INSERT(); // <INSERT
}


// ================================================
// 0x7c23: WORD 'IEXTRAC' codep=0x224c wordp=0x7c2f params=0 returns=2
// ================================================

void IEXTRAC() // IEXTRAC
{
  CI(); // CI
  W7635(); // W7635
  if (Pop() != 0)
  {
    ICLOSE(); // ICLOSE
    Push2Words("NULL");
    W74E0(); // W74E0
    IOPEN(); // IOPEN
    return;
  }
  IsFIRST(); // ?FIRST
  if (Pop() != 0)
  {
    W750A(); // W750A
    W7502(); // W7502
    ICLOSE(); // ICLOSE
    W74E0(); // W74E0
    IOPEN(); // IOPEN
    W74D8(); // W74D8
    CI(); // CI
    IPREV(); // IPREV
    W74D0(); // W74D0
    INEXT(); // INEXT
    return;
  }
  W7502(); // W7502
  IPREV(); // IPREV
  W74D0(); // W74D0
  CI(); // CI
  INEXT(); // INEXT
  W74D8(); // W74D8
}


// ================================================
// 0x7c6f: WORD 'W7C71' codep=0x224c wordp=0x7c71 params=0 returns=2
// ================================================

void W7C71() // W7C71
{
  Push2Words("VANEWSP");
  VA_gt_BUF(); // VA>BUF
}


// ================================================
// 0x7c77: WORD '@NEWSPA' codep=0x224c wordp=0x7c83 params=0 returns=3
// ================================================

void GetNEWSPA() // @NEWSPA
{
  W7C71(); // W7C71
  _1_dot_5_at_(); // 1.5@
}


// ================================================
// 0x7c89: WORD '!NEWSPA' codep=0x224c wordp=0x7c95 params=1 returns=0
// ================================================

void StoreNEWSPA() // !NEWSPA
{
  W7C71(); // W7C71
  _1_dot_5_ex__2(); // 1.5!_2
}


// ================================================
// 0x7c9b: WORD 'W7C9D' codep=0x224c wordp=0x7c9d params=0 returns=1
// ================================================

void W7C9D() // W7C9D
{
  GetNEWSPA(); // @NEWSPA
  ROT(); // ROT
  M_plus_(); // M+
  StoreNEWSPA(); // !NEWSPA
}


// ================================================
// 0x7ca7: WORD 'MAXSPAC' codep=0x224c wordp=0x7cb3 params=0 returns=2
// ================================================

void MAXSPAC() // MAXSPAC
{
  Push2Words("VANEWSP");
  Push(0x0003); Push(0x0000);
  D_plus_(); // D+
  VA_gt_BUF(); // VA>BUF
}


// ================================================
// 0x7cc1: WORD 'W7CC3' codep=0x224c wordp=0x7cc3 params=0 returns=2
// ================================================

void W7CC3() // W7CC3
{
  GetNEWSPA(); // @NEWSPA
  VA_gt_BLK(); // VA>BLK
  Pop(); // DROP
  Push(0x0400);
  SWAP(); // SWAP
  _dash_(); // -
}


// ================================================
// 0x7cd3: WORD 'FLD@' codep=0x4b3b wordp=0x7cdc
// ================================================

void FLD_at_() // FLD@
{
  switch(Pop()) // FLD@
  {
  case 1:
    Push(Read8(Pop())&0xFF); // C@
    break;
  case 2:
    Push(Read16(Pop())); // @
    break;
  case 3:
    _1_dot_5_at_(); // 1.5@
    break;
  default:
    Push(Read16(Pop())); // @
    break;

  }
}

// ================================================
// 0x7cec: WORD 'FLD!' codep=0x4b3b wordp=0x7cf5
// ================================================

void FLD_ex_() // FLD!
{
  switch(Pop()) // FLD!
  {
  case 1:
    C_ex__2(); // C!_2
    break;
  case 2:
    Store_2(); // !_2
    break;
  case 3:
    _1_dot_5_ex__2(); // 1.5!_2
    break;
  default:
    Store_2(); // !_2
    break;

  }
}

// ================================================
// 0x7d05: WORD 'IFLD@' codep=0x224c wordp=0x7d0f
// ================================================

void IFLD_at_() // IFLD@
{
  Push(Read16(regsp)); // DUP
  Push(Read16(Pop() + 2)&0xFF); //  2+ C@
  SWAP(); // SWAP
  EXECUTE(); // EXECUTE
  SWAP(); // SWAP
  FLD_at_(); // FLD@ case
}


// ================================================
// 0x7d1f: WORD 'IFLD!' codep=0x224c wordp=0x7d29
// ================================================

void IFLD_ex_() // IFLD!
{
  Push(Read16(regsp)); // DUP
  Push(Read16(Pop() + 2)&0xFF); //  2+ C@
  SWAP(); // SWAP
  EXECUTE(); // EXECUTE
  SWAP(); // SWAP
  FLD_ex_(); // FLD! case
}


// ================================================
// 0x7d39: WORD 'W7D3B' codep=0x224c wordp=0x7d3b
// ================================================

void W7D3B() // W7D3B
{
  OVER(); // OVER
  IFLD_at_(); // IFLD@
  OVER(); // OVER
  Push((Pop()==Pop())?1:0); // =
}


// ================================================
// 0x7d45: WORD '?CLASS/' codep=0x224c wordp=0x7d51 params=2 returns=3
// ================================================

void IsCLASS_slash_() // ?CLASS/
{
  OVER(); // OVER
  Push(Read16(regsp)); // DUP
  Push(Pop()==0?1:0); //  0=
  SWAP(); // SWAP
  GetINST_dash_C(); // @INST-C
  Push((Pop()==Pop())?1:0); // =
  Push(Pop() | Pop()); // OR
  OVER(); // OVER
  Push(Read16(regsp)); // DUP
  Push(Pop()==0?1:0); //  0=
  SWAP(); // SWAP
  GetINST_dash_S(); // @INST-S
  Push((Pop()==Pop())?1:0); // =
  Push(Pop() | Pop()); // OR
  Push(Pop() & Pop()); // AND
}


// ================================================
// 0x7d71: WORD 'W7D73' codep=0x224c wordp=0x7d73
// ================================================

void W7D73() // W7D73
{
  Push(pp_W56B2); // W56B2
  GetEXECUTE(); // @EXECUTE
}


// ================================================
// 0x7d79: WORD 'W7D7B' codep=0x224c wordp=0x7d7b
// ================================================

void W7D7B() // W7D7B
{
  Push(pp__i__ask_EXIT); // '?EXIT
  GetEXECUTE(); // @EXECUTE
}


// ================================================
// 0x7d81: WORD 'W7D83' codep=0x224c wordp=0x7d83
// ================================================

void W7D83() // W7D83
{
  Push(pp__i_TRAVER); // 'TRAVER
  GetEXECUTE(); // @EXECUTE
}


// ================================================
// 0x7d89: WORD 'MAP' codep=0x224c wordp=0x7d91
// ================================================

void MAP() // MAP
{
  Push(pp__i_MAP); // 'MAP
  GetEXECUTE(); // @EXECUTE
}


// ================================================
// 0x7d97: WORD 'W7D99' codep=0x224c wordp=0x7d99 params=2 returns=0
// ================================================

void W7D99() // W7D99
{
  Push(Read16(regsp)); // DUP
  Push(Read16(Pop())); //  @
  _gt_V(); // >V
  _st__ex__gt_(); // <!>
}


// ================================================
// 0x7da3: WORD 'W7DA5' codep=0x224c wordp=0x7da5 params=1 returns=0
// ================================================

void W7DA5() // W7DA5
{
  V_gt_(); // V>
  SWAP(); // SWAP
  _st__ex__gt_(); // <!>
}


// ================================================
// 0x7dad: WORD 'SELECT' codep=0x224c wordp=0x7db8
// ================================================

void SELECT() // SELECT
{
  Push(0);
  _gt_V(); // >V
  while(1)
  {
    W7D73(); // W7D73
    W7D7B(); // W7D7B
    Push(Pop() | Pop()); // OR
    Push(!Pop()); //  NOT
    if (Pop() == 0) break;

    W7D83(); // W7D83
    V_gt_(); // V>
    Pop(); // DROP
    Push(1);
    _gt_V(); // >V
  }
  W7D7B(); // W7D7B
  Push(!Pop()); //  NOT
  V_gt_(); // V>
  Pop(); // DROP
}


// ================================================
// 0x7de0: WORD '?>FIRST' codep=0x224c wordp=0x7dec params=0 returns=1
// ================================================

void Is_gt_FIRST() // ?>FIRST
{
  VI(); // VI
  IsFIRST(); // ?FIRST
  Push(Pop() & Pop()); // AND
  IsNULL(); // ?NULL
  Push(Pop() | Pop()); // OR
}


// ================================================
// 0x7df8: WORD 'SELECT-' codep=0x224c wordp=0x7e04
// ================================================

void SELECT_dash_() // SELECT-
{
  unsigned short int a;
  Push(pp_W56B2); // W56B2
  W7D99(); // W7D99
  Push(0x7dec); // '?>FIRST'
  Push(pp__i__ask_EXIT); // '?EXIT
  W7D99(); // W7D99
  Push(0x7ab3); // 'INEXT'
  Push(pp__i_TRAVER); // 'TRAVER
  W7D99(); // W7D99
  SELECT(); // SELECT
  Push(pp__i_TRAVER); // 'TRAVER
  W7DA5(); // W7DA5
  Push(pp__i__ask_EXIT); // '?EXIT
  W7DA5(); // W7DA5
  Push(pp_W56B2); // W56B2
  W7DA5(); // W7DA5
  a = Pop(); // >R
  Pop(); Pop(); // 2DROP
  Push(a); // R>
}


// ================================================
// 0x7e2e: WORD 'W7E30' codep=0x224c wordp=0x7e30
// ================================================
// orphan

void W7E30() // W7E30
{
  unsigned short int a;
  SELECT_dash_(); // SELECT-
  a = Pop(); // >R
  Pop(); // DROP
  Push(a); // R>
}


// ================================================
// 0x7e3a: WORD 'W7E3C' codep=0x224c wordp=0x7e3c
// ================================================

void W7E3C() // W7E3C
{
  Push(0x7d3b); // 'W7D3B'
  SELECT_dash_(); // SELECT-
}


// ================================================
// 0x7e44: WORD 'IFIND' codep=0x224c wordp=0x7e4e
// ================================================

void IFIND() // IFIND
{
  Push(0x7d51); // '?CLASS/'
  SELECT_dash_(); // SELECT-
}


// ================================================
// 0x7e56: WORD 'W7E58' codep=0x224c wordp=0x7e58 params=0 returns=0
// ================================================

void W7E58() // W7E58
{
  IsCHILD(); // ?CHILD
  if (Pop() == 0) return;
  IOPEN(); // IOPEN
  do
  {
    W7E58(); // W7E58
    MAP(); // MAP
    W7D83(); // W7D83
    W7D7B(); // W7D7B
  } while(Pop() == 0);
  ICLOSE(); // ICLOSE
}


// ================================================
// 0x7e70: WORD 'MAP>LEA' codep=0x224c wordp=0x7e7c
// ================================================

void MAP_gt_LEA() // MAP>LEA
{
  MAP(); // MAP
  IsCHILD(); // ?CHILD
  if (Pop() == 0) return;
  IOPEN(); // IOPEN
  do
  {
    MAP_gt_LEA(); // MAP>LEA
    W7D83(); // W7D83
    W7D7B(); // W7D7B
  } while(Pop() == 0);
  ICLOSE(); // ICLOSE
}


// ================================================
// 0x7e94: WORD 'MAKE1ST' codep=0x224c wordp=0x7ea0 params=0 returns=0
// ================================================

void MAKE1ST() // MAKE1ST
{
  IsFIRST(); // ?FIRST
  Push(!Pop()); //  NOT
  if (Pop() == 0) return;
  IEXTRAC(); // IEXTRAC
  CI_i_(); // CI'
  IINSERT(); // IINSERT
  IFIRST(); // IFIRST
}


// ================================================
// 0x7eb2: WORD 'W7EB4' codep=0x224c wordp=0x7eb4 params=3 returns=0
// ================================================

void W7EB4() // W7EB4
{
  Push2Words("INACTIV");
  _gt_C_plus_S(); // >C+S
  IsCHILD(); // ?CHILD
  if (Pop() != 0)
  {
    IOPEN(); // IOPEN
    Push(0x74a0); // 'INST-SP'
    SWAP(); // SWAP
    W7E3C(); // W7E3C
    if (Pop() != 0)
    {
      MAKE1ST(); // MAKE1ST
      CI(); // CI
    } else
    {
      CI_i_(); // CI'
    }
    ICLOSE(); // ICLOSE
  } else
  {
    Pop(); // DROP
    CI(); // CI
  }
  IINSERT(); // IINSERT
  ICLOSE(); // ICLOSE
}


// ================================================
// 0x7ee6: WORD '>INACTI' codep=0x224c wordp=0x7ef2 params=2 returns=0
// ================================================

void _gt_INACTI() // >INACTI
{
  _2DUP(); // 2DUP
  _gt_C_plus_S(); // >C+S
  Push((Read16(pp_IBFR) - 3) - Read16(cc_IHEADLE)); // IBFR @ 3 - IHEADLE -
  Push(Read16(regsp)); // DUP
  StoreINST_dash_S(); // !INST-S
  CDROP(); // CDROP
  W7EB4(); // W7EB4
}


// ================================================
// 0x7f0c: WORD 'W7F0E' codep=0x224c wordp=0x7f0e params=0 returns=0
// ================================================

void W7F0E() // W7F0E
{
  IEXTRAC(); // IEXTRAC
  _gt_INACTI(); // >INACTI
}


// ================================================
// 0x7f14: WORD 'IDELETE' codep=0x224c wordp=0x7f20
// ================================================

void IDELETE() // IDELETE
{
  Push(0x7f0e); // 'W7F0E'
  Push(pp__i_MAP); // 'MAP
  W7D99(); // W7D99
  Push(0x7609); // '?NULL'
  Push(pp__i__ask_EXIT); // '?EXIT
  W7D99(); // W7D99
  Push(0x3a48); // 'NOP'
  Push(pp__i_TRAVER); // 'TRAVER
  W7D99(); // W7D99
  W7E58(); // W7E58
  MAP(); // MAP
  Push(pp__i_TRAVER); // 'TRAVER
  W7DA5(); // W7DA5
  Push(pp__i__ask_EXIT); // '?EXIT
  W7DA5(); // W7DA5
  Push(pp__i_MAP); // 'MAP
  W7DA5(); // W7DA5
}


// ================================================
// 0x7f4a: WORD 'W7F4C' codep=0x224c wordp=0x7f4c
// ================================================

void W7F4C() // W7F4C
{
  Push(pp__i_MAP); // 'MAP
  W7D99(); // W7D99
  Push(pp__i_TRAVER); // 'TRAVER
  W7D99(); // W7D99
  Push(pp__i__ask_EXIT); // '?EXIT
  W7D99(); // W7D99
  Push(pp_W56B2); // W56B2
  W7D99(); // W7D99
  IFIRST(); // IFIRST
  do
  {
    SELECT(); // SELECT
    if (Pop() != 0)
    {
      MAP(); // MAP
      W7D83(); // W7D83
      IsFIRST(); // ?FIRST
    } else
    {
      Push(1);
    }
  } while(Pop() == 0);
  Push(pp_W56B2); // W56B2
  W7DA5(); // W7DA5
  Push(pp__i__ask_EXIT); // '?EXIT
  W7DA5(); // W7DA5
  Push(pp__i_TRAVER); // 'TRAVER
  W7DA5(); // W7DA5
  Push(pp__i_MAP); // 'MAP
  W7DA5(); // W7DA5
}


// ================================================
// 0x7f86: WORD 'ALL' codep=0x224c wordp=0x7f8e
// ================================================

void ALL() // ALL
{
  unsigned short int a;
  a = Pop(); // >R
  Push(0x761a); // '?-NULL'
  Push(0x7dec); // '?>FIRST'
  Push(0x7ab3); // 'INEXT'
  Push(a); // R>
  W7F4C(); // W7F4C
}


// ================================================
// 0x7fa2: WORD 'EACH' codep=0x224c wordp=0x7fab
// ================================================

void EACH() // EACH
{
  unsigned short int a;
  a = Pop(); // >R
  Push(0x7dec); // '?>FIRST'
  Push(0x7ab3); // 'INEXT'
  Push(a); // R>
  W7F4C(); // W7F4C
}


// ================================================
// 0x7fbb: WORD 'W7FBD' codep=0x224c wordp=0x7fbd
// ================================================

void W7FBD() // W7FBD
{
  Push(pp__i_THROW_dash_); // 'THROW-
  GetEXECUTE(); // @EXECUTE
}


// ================================================
// 0x7fc3: WORD 'NULLPOI' codep=0x224c wordp=0x7fcf params=0 returns=0
// ================================================

void NULLPOI() // NULLPOI
{
  Push2Words("NULL");
  W74D0(); // W74D0
  Push2Words("NULL");
  W74D8(); // W74D8
  Push2Words("NULL");
  W74E0(); // W74E0
}


// ================================================
// 0x7fdd: WORD 'W7FDF' codep=0x224c wordp=0x7fdf params=0 returns=2
// ================================================

void W7FDF() // W7FDF
{
  IsCHILD(); // ?CHILD
  if (Pop() != 0)
  {
    IOPEN(); // IOPEN
    IEXTRAC(); // IEXTRAC
    ICLOSE(); // ICLOSE
    return;
  }
  IEXTRAC(); // IEXTRAC
}


// ================================================
// 0x7ff3: WORD 'W7FF5' codep=0x224c wordp=0x7ff5 params=0 returns=0
// ================================================

void W7FF5() // W7FF5
{
  Push2Words("INACTIV");
  _gt_C_plus_S(); // >C+S
  IsCHILD(); // ?CHILD
  if (Pop() != 0)
  {
    IOPEN(); // IOPEN
    Push(0x74a0); // 'INST-SP'
    OVER(); // OVER
    W7E3C(); // W7E3C
    if (Pop() != 0)
    {
      Push(pp_W5FE6); // W5FE6
      ON_1(); // ON_1
      MAKE1ST(); // MAKE1ST
      W7FDF(); // W7FDF
    } else
    {
      Push2Words("NULL");
    }
    ICLOSE(); // ICLOSE
  } else
  {
    Push2Words("NULL");
  }
  ICLOSE(); // ICLOSE
  _gt_C(); // >C
}


// ================================================
// 0x8029: WORD 'W802B' codep=0x224c wordp=0x802b params=0 returns=4
// ================================================

void W802B() // W802B
{
  unsigned short int a;
  W7CC3(); // W7CC3
  Push(Read16(cc_IHEADLE)); // IHEADLE
  _gt_(); // >
  if (Pop() != 0)
  {
    W7CC3(); // W7CC3
    Push((Pop() - Read16(cc_IHEADLE)) - 1); //  IHEADLE - 1-
    GetNEWSPA(); // @NEWSPA
    VA_gt_BUF(); // VA>BUF
    a = Pop(); // >R
    Push(a + 0x000b); // I 0x000b +
    C_ex__2(); // C!_2
    Push(0x0030);
    Push(a + 9); // R> 9 +
    C_ex__2(); // C!_2
    GetNEWSPA(); // @NEWSPA
    _gt_C_plus_S(); // >C+S
    NULLPOI(); // NULLPOI
    CI(); // CI
    ICLOSE(); // ICLOSE
    _gt_INACTI(); // >INACTI
  }
  W7CC3(); // W7CC3
  W7C9D(); // W7C9D
}


// ================================================
// 0x806b: WORD '?>MAXSP' codep=0x224c wordp=0x8077 params=1 returns=4
// ================================================

void Is_gt_MAXSP() // ?>MAXSP
{
  unsigned short int a;
  Push(Read16(regsp)); // DUP
  a = Pop(); // >R
  MAXSPAC(); // MAXSPAC
  _1_dot_5_at_(); // 1.5@
  GetNEWSPA(); // @NEWSPA
  Push(a); // R>
  Push(0);
  D_plus_(); // D+
  D_st_(); // D<
}


// ================================================
// 0x808b: WORD 'W808D' codep=0x224c wordp=0x808d params=1 returns=9
// ================================================

void W808D() // W808D
{
  Is_gt_MAXSP(); // ?>MAXSP
  if (Pop() != 0)
  {
    Push2Words("NULL");
    _gt_C(); // >C
    return;
  }
  W7CC3(); // W7CC3
  OVER(); // OVER
  Push(Pop() + Read16(cc_IHEADLE)); //  IHEADLE +
  _st_(); // <
  if (Pop() != 0)
  {
    W802B(); // W802B
  }
  GetNEWSPA(); // @NEWSPA
  _gt_C(); // >C
  Push(Read16(regsp)); // DUP
  Push(Pop() + Read16(cc_IHEADLE)); //  IHEADLE +
  W7C9D(); // W7C9D
  Is_gt_MAXSP(); // ?>MAXSP
  if (Pop() == 0) return;
  CDROP(); // CDROP
  Push2Words("NULL");
  _gt_C(); // >C
}


// ================================================
// 0x80c5: WORD 'W80C7' codep=0x224c wordp=0x80c7 params=1 returns=9
// ================================================

void W80C7() // W80C7
{
  while(1)
  {
    Push(pp_W5FE6); // W5FE6
    OFF_2(); // OFF_2
    Push(Read16(pp_W5EFA)); // W5EFA @
    if (Pop() != 0)
    {
      W7FF5(); // W7FF5
    } else
    {
      W808D(); // W808D
    }
    IsNULL(); // ?NULL
    if (Pop() != 0)
    {
      CDROP(); // CDROP
      Push(Read16(pp_W5EFA)); // W5EFA @
      if (Pop() != 0)
      {
        W808D(); // W808D
      } else
      {
        W7FF5(); // W7FF5
      }
      IsNULL(); // ?NULL
      if (Pop() != 0)
      {
        W7FBD(); // W7FBD
      }
    }
    IsNULL(); // ?NULL
    if (Pop() == 0) return;
    CDROP(); // CDROP
  }
}


// ================================================
// 0x8109: WORD 'SET?REU' codep=0x4b3b wordp=0x8115
// ================================================

void SET_ask_REU() // SET?REU
{
  switch(Pop()) // SET?REU
  {
  case 11:
    Func9("<ON>");
    break;
  case 41:
    Func9("<ON>");
    break;
  case 14:
    Func9("<ON>");
    break;
  case 26:
    Func9("<ON>");
    break;
  case 68:
    Func9("<ON>");
    break;
  case 28:
    Func9("<ON>");
    break;
  case 56:
    Func9("<ON>");
    break;
  case 36:
    Func9("<ON>");
    break;
  case 24:
    Func9("<ON>");
    break;
  case 25:
    Func9("<ON>");
    break;
  case 67:
    Func9("<ON>");
    break;
  case 21:
    Func9("<ON>");
    break;
  case 9:
    Func9("<ON>");
    break;
  case 31:
    Func9("<ON>");
    break;
  case 33:
    Func9("<ON>");
    break;
  case 29:
    Func9("<ON>");
    break;
  default:
    Func9("<OFF>");
    break;

  }
}

// ================================================
// 0x8159: WORD 'VICREAT' codep=0x224c wordp=0x8165 params=1 returns=11
// ================================================

void VICREAT() // VICREAT
{
  unsigned short int a;
  Push(pp_W5EFA); // W5EFA
  OFF_2(); // OFF_2
  Push(Read16(regsp)); // DUP
  Push(0x00fe);
  _gt_(); // >
  IsUNRAVEL(); // ?UNRAVEL
  Push(Pop() + 1); //  1+
  W80C7(); // W80C7
  Push(Pop() - 1); //  1-
  CI(); // CI
  VA_gt_BUF(); // VA>BUF
  a = Pop(); // >R
  Push(a + 0x000b); // I 0x000b +
  C_ex__2(); // C!_2
  Push(0x0030);
  Push(a + 9); // R> 9 +
  C_ex__2(); // C!_2
  SET_dash_CUR(); // SET-CUR
  NULLPOI(); // NULLPOI
  CI(); // CI
  ICLOSE(); // ICLOSE
}


// ================================================
// 0x819f: WORD 'ICREATE' codep=0x224c wordp=0x81ab
// ================================================

void ICREATE() // ICREATE
{
  unsigned short int a;
  Push(pp_W5EFA); // W5EFA
  Push(3);
  PICK(); // PICK
  SET_ask_REU(); // SET?REU case
  OVER(); // OVER
  Func6("FILE-SL");
  Push(Read16(Pop())&0xFF); //  C@
  W80C7(); // W80C7
  Pop(); // DROP
  Push(!Read16(pp_W5FE6)); // W5FE6 @ NOT
  if (Pop() != 0)
  {
    CI(); // CI
    VA_gt_BUF(); // VA>BUF
    Push(Read16(regsp)); // DUP
    a = Pop(); // >R
    Push(Pop() + 0x000a); //  0x000a +
    C_ex__2(); // C!_2
    Push(a + 9); // R> 9 +
    C_ex__2(); // C!_2
    SET_dash_CUR(); // SET-CUR
  } else
  {
    SET_dash_CUR(); // SET-CUR
    StoreINST_dash_S(); // !INST-S
    W74E8(); // W74E8
  }
  NULLPOI(); // NULLPOI
  CI(); // CI
  ICLOSE(); // ICLOSE
}


// ================================================
// 0x81f3: WORD 'BOX-NAM' codep=0x7394 wordp=0x81ff
// ================================================
LoadDataType BOX_dash_NAM = {BOXIDX, 0x00, 0x10, 0x10, 0x6a7d};

// ================================================
// 0x8205: WORD '*CREATE' codep=0x224c wordp=0x8211
// ================================================

void _star_CREATE() // *CREATE
{
  unsigned short int i, imax;

  i = 0;
  imax = Pop();
  do // (DO)
  {
    _2DUP(); // 2DUP
    ICREATE(); // ICREATE
    IsNULL(); // ?NULL
    if (Pop() != 0)
    {
      CI_i_(); // CI'
      IINSERT(); // IINSERT
      IFIRST(); // IFIRST
    } else
    {
      W7A6C(); // W7A6C
      _st_INSERT(); // <INSERT
    }
    i++;
  } while(i<imax); // (LOOP)

  ILAST(); // ILAST
  Pop(); Pop(); // 2DROP
}


// ================================================
// 0x8237: WORD '.C' codep=0x224c wordp=0x823e params=0 returns=0
// ================================================

void DrawC() // .C
{
  unsigned short int i, imax;
  Exec("CR"); // call of word 0x26ee '(CR)'
  CDEPTH(); // CDEPTH
  if (Pop() != 0)
  {

    i = Read16(cc_W50FA); // W50FA
    imax = Read16(pp_CXSP) + 3; // CXSP @ 3 +
    do // (DO)
    {
      Push(i); // I
      _1_dot_5_at_(); // 1.5@
      Push(2);
      SPACES(); // SPACES
      Push(5);
      D_dot_R(); // D.R
      Push(-3);
      int step = Pop();
      i += step;
      if (((step>=0) && (i>=imax)) || ((step<0) && (i<=imax))) break;
    } while(1); // (+LOOP)

  } else
  {
    PRINT("MT STK", 6); // (.")
  }
  Exec("CR"); // call of word 0x26ee '(CR)'
}


// ================================================
// 0x8277: WORD 'W8279' codep=0x224c wordp=0x8279 params=0 returns=1
// ================================================

void W8279() // W8279
{
  Push(Read16(pp_OVA)); // OVA @
}


// ================================================
// 0x827f: WORD 'OV-CANC' codep=0x224c wordp=0x828b params=0 returns=0
// ================================================

void OV_dash_CANC() // OV-CANC
{
  Push(Read16(pp_OV_n_)); // OV# @
  if (Pop() != 0)
  {
    W8279(); // W8279
    Push(Read16(regsp)); // DUP
    Push(4);
    _plus__at_(); // +@
    Push((Pop()==Pop())?1:0); // =
    if (Pop() != 0)
    {
      W8279(); // W8279
      Push(8);
      _plus__at_(); // +@
      Push(Pop() + 4); //  4 +
      Push(8);
      Push(0);
      FILL_2(); // FILL_2
    }
  }
  Push(pp_OV_n_); // OV#
  OFF_2(); // OFF_2
  Push(0x081a); // 'FORTH'
  Push(Read16(regsp)); // DUP
  Push(user_CURRENT); // CURRENT
  _st__ex__gt_(); // <!>
  Push(user_CONTEXT_1); // CONTEXT_1
  _st__ex__gt_(); // <!>
}


// ================================================
// 0x82c5: WORD 'W82C7' codep=0x224c wordp=0x82c7 params=3 returns=2
// ================================================

void W82C7() // W82C7
{
  Push(Read16(regsp)); // DUP
  W700F(); // W700F
  W7075(); // W7075
  VA_gt_BUF(); // VA>BUF
}


// ================================================
// 0x82d1: WORD 'W82D3' codep=0x224c wordp=0x82d3 params=1 returns=0
// ================================================

void W82D3() // W82D3
{
  HERE(); // HERE
  U_st_(); // U<
  if (Pop() == 0) return;
  PRINT("OV TOO BIG", 10); // (.")
  UNRAVEL(); // UNRAVEL
}


// ================================================
// 0x82ec: WORD 'W82EE' codep=0x224c wordp=0x82ee params=2 returns=0
// ================================================

void W82EE() // W82EE
{
  Push(Read16(regsp)); // DUP
  W82C7(); // W82C7
  _2DUP(); // 2DUP
  Push(Read16(Pop())); //  @
  _dash_(); // -
  if (Pop() != 0)
  {
    PRINT("INV OV BLK", 10); // (.")
    UNRAVEL(); // UNRAVEL
  }
  Push(4);
  _plus__at_(); // +@
  W82D3(); // W82D3
  Pop(); // DROP
}


// ================================================
// 0x8315: WORD 'W8317' codep=0x224c wordp=0x8317 params=1 returns=0
// ================================================

void W8317() // W8317
{
  unsigned short int a;
  if (Read16(regsp) != 0) Push(Read16(regsp)); // ?DUP
  if (Pop() == 0) return;
  Push(Read16(regsp)); // DUP
  Push(Pop()==Read16(pp_OV_n_)?1:0); //  OV# @ =
  if (Pop() != 0)
  {
    Pop(); // DROP
    return;
  }
  Push(Read16(regsp)); // DUP
  W82EE(); // W82EE
  OV_dash_CANC(); // OV-CANC
  W82C7(); // W82C7
  a = Pop(); // >R
  Push(a); // I
  Push(4);
  _plus__at_(); // +@
  Push(Read16(regsp)); // DUP
  Push(pp_OVA); // OVA
  _st__ex__gt_(); // <!>
  ADDR_gt_SEG(); // ADDR>SEG
  Push(Read16(a)); // I @
  Push(Read16(regsp)); // DUP
  Push(pp_OV_n_); // OV#
  _st__ex__gt_(); // <!>
  Push(Read16(a + 2)); // R> 2+ @
  MEM_st_DSK(); // MEM<DSK
  W8279(); // W8279
  Push(Pop() + 0x000a); //  0x000a +
  W8279(); // W8279
  Push(8);
  _plus__at_(); // +@
  Push(Pop() + 4); //  4 +
  Push(8);
  CMOVE_2(); // CMOVE_2
  W8279(); // W8279
  Push(8);
  _plus__at_(); // +@
  Push(Pop() - 2); //  2-
  Push(Read16(regsp)); // DUP
  Push(user_CONTEXT_1); // CONTEXT_1
  _st__ex__gt_(); // <!>
  Push(user_CURRENT); // CURRENT
  _st__ex__gt_(); // <!>
}


// ================================================
// 0x8383: WORD 'SAVE-OV' codep=0x224c wordp=0x838f params=0 returns=0
// ================================================

void SAVE_dash_OV() // SAVE-OV
{
  unsigned short int a;
  Push(Read16(pp_OV_n_)); // OV# @
  if (Pop() == 0) return;
  W8279(); // W8279
  a = Pop(); // >R
  Push(a); // I
  Push(8);
  _plus__at_(); // +@
  Push(Pop() + 4); //  4 +
  Push(a + 0x000a); // I 0x000a +
  Push(8);
  CMOVE_2(); // CMOVE_2
  Push(a); // I
  ADDR_gt_SEG(); // ADDR>SEG
  Push(Read16(a)); // I @
  Push(Read16(a + 2)); // R> 2+ @
  MEM_gt_DSK(); // MEM>DSK
}


// ================================================
// 0x83c3: WORD 'OVERLAY' codep=0x224c wordp=0x83cf
// ================================================

void OVERLAY() // OVERLAY
{
  Exec("CREATE"); // call of word 0x1cbb '(CREATE)'
  Func6("FILE-ST");
  Push(Read16(Pop())); //  @
  Push(Read16(regsp)); // DUP
  _co_(); // ,
  W82EE(); // W82EE
  CODE(); // (;CODE) inlined assembler code
// 0x83dd: call   1649
  Push(Read16(Pop())); //  @
  W8317(); // W8317
}


// ================================================
// 0x83e6: WORD 'W83E8' codep=0x224c wordp=0x83e8 params=0 returns=0
// ================================================

void W83E8() // W83E8
{
  Push(Read16(user_DP)); // DP @
  W8279(); // W8279
  Push(6);
  _plus__at_(); // +@
  Push(user_DP); // DP
  _st__ex__gt_(); // <!>
  W8279(); // W8279
  Push(Pop() + 6); //  6 +
  _st__ex__gt_(); // <!>
}


// ================================================
// 0x8400: WORD 'OPEN-OV' codep=0x224c wordp=0x840c
// ================================================

void OPEN_dash_OV() // OPEN-OV
{
  Push(Read16(regsp)); // DUP
  Func6("FILE-ST");
  Push(Read16(Pop())); //  @
  SWAP(); // SWAP
  Func6("FILE-#R");
  Push(Read16(Pop())); //  @
  OVER(); // OVER
  Push(pp_OV_n_); // OV#
  _st__ex__gt_(); // <!>
  Push(Read16(regsp)); // DUP
  Push(-(Pop() >> 4) + Read16(cc_W5102)); //  16* NEGATE W5102 +
  Push(Read16(regsp)); // DUP
  W82D3(); // W82D3
  Push(Read16(regsp)); // DUP
  Push(pp_OVA); // OVA
  _st__ex__gt_(); // <!>
  HERE(); // HERE
  OVER(); // OVER
  Push(Pop() + 6); //  6 +
  _st__ex__gt_(); // <!>
  Push(user_DP); // DP
  _st__ex__gt_(); // <!>
  SWAP(); // SWAP
  _co_(); // ,
  _co_(); // ,
  W8279(); // W8279
  _co_(); // ,
  Push(2);
  ALLOT(); // ALLOT
  Push(Read16(user_VOC_dash_LINK)); // VOC-LINK @
  _co_(); // ,
  Push(8);
  ALLOT(); // ALLOT
  Push(2);
  ALLOT(); // ALLOT
}


// ================================================
// 0x845e: WORD 'W8460' codep=0x224c wordp=0x8460
// ================================================

void W8460() // W8460
{
  Exec("CR"); // call of word 0x26ee '(CR)'
  Push(Read16(user_CONTEXT_1)); // CONTEXT_1 @
  NFA(); // NFA
  ID_dot_(); // ID.
  W8279(); // W8279
  HERE(); // HERE
  OVER(); // OVER
  _dash_(); // -
  Push((Pop() >> 4) + 1); //  16/ 1+
  Push(Read16(regsp)); // DUP
  Draw(); // .
  PRINT("SEGS USED", 9); // (.")
  W8279(); // W8279
  Push(Read16(Pop() + 2)); //  2+ @
  OVER(); // OVER
  _dash_(); // -
  Push(Read16(regsp)); // DUP
  Push(3);
  SPACES(); // SPACES
  Draw(); // .
  PRINT("free", 4); // (.")
  Exec("CR"); // call of word 0x26ee '(CR)'
  _0_st_(); // 0<
  ABORT("#SEGS!", 6);// (ABORT")
  SWAP(); // SWAP
  Push(Pop() + 0x0012); //  0x0012 +
  _st__ex__gt_(); // <!>
}


// ================================================
// 0x84b8: WORD 'CLOSE-O' codep=0x224c wordp=0x84c4
// ================================================

void CLOSE_dash_O() // CLOSE-O
{
  W8460(); // W8460
  W83E8(); // W83E8
  SAVE_dash_OV(); // SAVE-OV
  SAVE_dash_BU(); // SAVE-BU
  OV_dash_CANC(); // OV-CANC
}


// ================================================
// 0x84d0: WORD 'MODULE' codep=0x224c wordp=0x84db params=1 returns=0
// ================================================

void MODULE() // MODULE
{
  Push(Read16(pp_OV_n_)); // OV# @
  _gt_V(); // >V
  SAVE_dash_OV(); // SAVE-OV
  EXECUTE(); // EXECUTE
  V_gt_(); // V>
  W8317(); // W8317
}


// ================================================
// 0x84eb: WORD 'GRSEG' codep=0x1d29 wordp=0x84f5
// ================================================
// 0x84f5: db 0x3e 0x95 '> '

// ================================================
// 0x84f7: WORD 'SHL-BIT' codep=0x8503 wordp=0x8503
// ================================================
// 0x8503: pop    ax
// 0x8504: pop    cx
// 0x8505: shr    ax,1
// 0x8507: rcl    cx,1
// 0x8509: push   cx
// 0x850a: lodsw
// 0x850b: mov    bx,ax
// 0x850d: jmp    word ptr [bx]

// ================================================
// 0x850f: WORD 'W8511' codep=0x1d29 wordp=0x8511
// ================================================
// 0x8511: db 0x00 0x06 0x05 0x07 0x03 '     '

// ================================================
// 0x8516: WORD '1PIX' codep=0x224c wordp=0x851f params=0 returns=0
// ================================================

void _1PIX() // 1PIX
{
  Push(0);
  Push(Read16(pp_GRSEG)); // GRSEG @
  Push(0x02b2);
  L_ex_(); // L!
}


// ================================================
// 0x852d: WORD '2PIX' codep=0x224c wordp=0x8536 params=0 returns=0
// ================================================

void _2PIX() // 2PIX
{
  Push(8);
  Push(Read16(pp_GRSEG)); // GRSEG @
  Push(0x02b2);
  L_ex_(); // L!
}


// ================================================
// 0x8544: WORD 'GRCALL' codep=0x854f wordp=0x854f
// ================================================
// 0x854f: mov    ax,[16B6] // REGISTERS
// 0x8553: mov    cx,[16BA]
// 0x8557: mov    dx,[16BC]
// 0x855b: mov    bx,[16B8]
// 0x855f: int    63
// 0x8561: mov    [16BC],dx
// 0x8565: mov    [16B6],ax // REGISTERS
// 0x8569: lodsw
// 0x856a: mov    bx,ax
// 0x856c: jmp    word ptr [bx]

// ================================================
// 0x856e: WORD 'PAD|16' codep=0x224c wordp=0x8579 params=0 returns=1
// ================================================

void PAD_v_16() // PAD|16
{
  PAD(); // PAD
  ADDR_gt_SEG(); // ADDR>SEG
  Push(Pop() + 1); //  1+
  SEG_gt_ADDR(); // SEG>ADDR
}


// ================================================
// 0x8583: WORD 'PAD>SEG' codep=0x224c wordp=0x858f params=0 returns=1
// ================================================

void PAD_gt_SEG() // PAD>SEG
{
  PAD_v_16(); // PAD|16
  ADDR_gt_SEG(); // ADDR>SEG
}


// ================================================
// 0x8595: WORD 'W8597' codep=0x224c wordp=0x8597
// ================================================

void W8597() // W8597
{
  Push(0x0014);
  Push(Read16(cc_AX)); // AX
  Store_2(); // !_2
  GRCALL(); // GRCALL
}


// ================================================
// 0x85a3: WORD '@MODE' codep=0x224c wordp=0x85ad
// ================================================

void GetMODE() // @MODE
{
  Push(0x0015);
  Push(Read16(cc_AX)); // AX
  Store_2(); // !_2
  GRCALL(); // GRCALL
  Push(Read16(Read16(cc_DX))); // DX @
  Push(Read16(regsp)); // DUP
  Push(pp_GRSEG); // GRSEG
  Store_2(); // !_2
  Push(0x010a);
  L_at_(); // L@
  Push(Read16(regsp)); // DUP
  Push(Read16((Pop() >> 1) + pp_W8511)&0xFF); //  2/ W8511 + C@
  Push(Read16(regsp)); // DUP
  Push(pp_MONITOR); // MONITOR
  Store_2(); // !_2
  Push(Pop()==5?1:0); //  5 =
  Push(pp_IsEGA); // ?EGA
  Store_2(); // !_2
}


// ================================================
// 0x85e1: WORD '?HERCUL' codep=0x224c wordp=0x85ed params=0 returns=1
// ================================================

void IsHERCUL() // ?HERCUL
{
  Push(Read16(pp_MONITOR)==4?1:0); // MONITOR @ 4 =
}


// ================================================
// 0x85f7: WORD '?VGA' codep=0x224c wordp=0x8600
// ================================================

void IsVGA() // ?VGA
{
  GetMODE(); // @MODE
  Push(Pop()==2?1:0); //  2 =
}


// ================================================
// 0x8608: WORD '?CGA' codep=0x224c wordp=0x8611
// ================================================

void IsCGA() // ?CGA
{
  GetMODE(); // @MODE
  Push(Pop()==8?1:0); //  8 =
}


// ================================================
// 0x8619: WORD 'CCGA' codep=0x1d29 wordp=0x8622
// ================================================
// 0x8622: db 0x00 0x01 0x01 0x01 0x02 0x02 0x02 0x03 0x00 0x01 0x01 0x01 0x02 0x02 0x03 0x03 '                '

// ================================================
// 0x8632: WORD 'PCGA' codep=0x1d29 wordp=0x863b
// ================================================
// 0x863b: db 0x00 0x02 0x02 0x02 0x02 0x02 0x02 0x01 0x00 0x01 0x01 0x03 0x01 0x01 0x03 0x03 '                '

// ================================================
// 0x864b: WORD 'NBCLR' codep=0x224c wordp=0x8655
// ================================================

void NBCLR() // NBCLR
{
  IsCGA(); // ?CGA
  if (Pop() != 0)
  {
    GetColor(RED);
    return;
  }
  GetColor(DK_dash_GREE);
}


// ================================================
// 0x8665: WORD '?XOR' codep=0x866e wordp=0x866e
// ================================================
// 0x866e: push   es
// 0x866f: push   word ptr [84F5] // GRSEG
// 0x8673: pop    es
// 0x8674: es:    
// 0x8675: mov    ax,[0108]
// 0x8679: mov    cx,[5A56] // XORMODE
// 0x867d: or     cx,cx
// 0x867f: jnz    8686
// 0x8681: and    ax,00FF
// 0x8684: jmp    8689
// 0x8686: and    ax,40FF
// 0x8689: es:    
// 0x868a: mov    [0108],ax
// 0x868e: pop    es
// 0x868f: lodsw
// 0x8690: mov    bx,ax
// 0x8692: jmp    word ptr [bx]

// ================================================
// 0x8694: WORD '!COLOR' codep=0x224c wordp=0x869f
// ================================================

void StoreCOLOR() // !COLOR
{
  Push(Read16(regsp)); // DUP
  Push(Read16(regsp)); // DUP
  Push(pp_COLOR); // COLOR
  Store_2(); // !_2
  Push(pp_DCOLOR); // DCOLOR
  Store_2(); // !_2
  IsCGA(); // ?CGA
  if (Pop() != 0)
  {
    Push(Read16(Pop() + pp_CCGA)&0xFF); //  CCGA + C@
  }
  Push(Read16(pp_XORMODE)); // XORMODE @
  if (Pop() != 0)
  {
    Push(Pop() | 0x4000); //  0x4000 OR
  } else
  {
    Push(Pop() & 0x00ff); //  0x00ff AND
  }
  Push(Read16(regsp)); // DUP
  Push(Read16(pp_GRSEG)); // GRSEG @
  Push(0x0108);
  L_ex_(); // L!
  Push(Read16(pp_GRSEG)); // GRSEG @
  Push(0x0106);
  L_ex_(); // L!
}


// ================================================
// 0x86e7: WORD '@COLOR' codep=0x224c wordp=0x86f2 params=0 returns=1
// ================================================

void GetCOLOR() // @COLOR
{
  Push(Read16(pp_COLOR)); // COLOR @
}


// ================================================
// 0x86f8: WORD 'W86FA' codep=0x224c wordp=0x86fa
// ================================================

void W86FA() // W86FA
{
  Push(pp_XORMODE); // XORMODE
  _099(); // 099
  IsXOR(); // ?XOR
}


// ================================================
// 0x8702: WORD 'RNDCLR' codep=0x224c wordp=0x870d
// ================================================

void RNDCLR() // RNDCLR
{
  Push(0);
  Push(0x0010);
  RRND(); // RRND
  Push(Read16(Pop() + pp_COLORT)&0xFF); //  COLORT + C@
  StoreCOLOR(); // !COLOR
}


// ================================================
// 0x871f: WORD 'W8721' codep=0x224c wordp=0x8721 params=0 returns=5
// ================================================

void W8721() // W8721
{
  Push(8);
  Push(0xa200);
  Push(0xa000);
  Push(Read16(cc__16K)); // 16K
  Push(0);
}


// ================================================
// 0x8731: WORD 'W8733' codep=0x224c wordp=0x8733 params=0 returns=5
// ================================================

void W8733() // W8733
{
  Push(2);
  Push(Read16(pp_XBUF_dash_SE)); // XBUF-SE @
  Push(0xb800);
  Push(Read16(cc__16K) * 2); // 16K 2*
  Push(0);
}


// ================================================
// 0x8745: WORD 'W8747' codep=0x224c wordp=0x8747 params=0 returns=5
// ================================================

void W8747() // W8747
{
  Push(2);
  Push(Read16(pp_XBUF_dash_SE)); // XBUF-SE @
  Push(0xa000);
  Push(Read16(cc__16K)); // 16K
  Push(4);
  M_star_(); // M*
}


// ================================================
// 0x8759: WORD 'W875B' codep=0x224c wordp=0x875b params=0 returns=5
// ================================================

void W875B() // W875B
{
  Push(2);
  Push(Read16(pp_XBUF_dash_SE)); // XBUF-SE @
  Push(0xb800);
  Push(Read16(cc__16K) * 2); // 16K 2*
  Push(0);
}


// ================================================
// 0x876d: WORD 'MON>MEM' codep=0x4b3b wordp=0x8779
// ================================================

void MON_gt_MEM() // MON>MEM
{
  switch(Pop()) // MON>MEM
  {
  case 4:
    W8721(); // W8721
    break;
  case 8:
    W8733(); // W8733
    break;
  case 6:
    W875B(); // W875B
    break;
  default:
    W8747(); // W8747
    break;

  }
}

// ================================================
// 0x8789: WORD 'SETDBUF' codep=0x224c wordp=0x8795
// ================================================

void SETDBUF() // SETDBUF
{
  Push(Read16(pp_REMSEG)); // REMSEG @
  Push(0x010a);
  L_at_(); // L@
  MON_gt_MEM(); // MON>MEM case
  Push(0x51b8); // 'DBUF-SI'
  StoreD(); // D!
  Push(pp_DBUF_dash_SE); // DBUF-SE
  Store_2(); // !_2
  Push(pp_HBUF_dash_SE); // HBUF-SE
  Store_2(); // !_2
  Push(pp_P_slash_B); // P/B
  Store_2(); // !_2
}


// ================================================
// 0x87b5: WORD 'W87B7' codep=0x1d29 wordp=0x87b7
// ================================================
// orphan
// 0x87b7: db 0x3a 0x20 0x05 0x41 ':  A'

// ================================================
// 0x87bb: WORD 'W87BD' codep=0x224c wordp=0x87bd params=0 returns=1
// ================================================
// orphan

void W87BD() // W87BD
{
  Push(Read16(pp_GRSEG)); // GRSEG @
  Push(0x0108);
  L_at_(); // L@
}


// ================================================
// 0x87c9: WORD 'W87CB' codep=0x224c wordp=0x87cb
// ================================================

void W87CB() // W87CB
{
  Push(Read16(regsp)); // DUP
  Push(Pop() & 0x00ff); //  0x00ff AND
  StoreCOLOR(); // !COLOR
  Push(Read16(regsp)); // DUP
  Push(Read16(pp_GRSEG)); // GRSEG @
  Push(0x0108);
  L_ex_(); // L!
  Push(0x0255);
  _gt_(); // >
  if (Pop() == 0) return;
  Push(pp_XORMODE); // XORMODE
  ON_2(); // ON_2
}


// ================================================
// 0x87f1: WORD 'L@PIXEL' codep=0x87fd wordp=0x87fd
// ================================================
// 0x87fd: pop    ax
// 0x87fe: xor    cx,cx
// 0x8800: mov    ch,al
// 0x8802: pop    bx
// 0x8803: mov    ax,0007
// 0x8806: int    63
// 0x8808: push   ax
// 0x8809: lodsw
// 0x880a: mov    bx,ax
// 0x880c: jmp    word ptr [bx]

// ================================================
// 0x880e: WORD 'LPLOT' codep=0x8818 wordp=0x8818 params=2 returns=0
// ================================================
// 0x8818: pop    ax
// 0x8819: pop    bx
// 0x881a: mov    bh,al
// 0x881c: push   es
// 0x881d: push   word ptr [84F5] // GRSEG
// 0x8821: pop    es
// 0x8822: es:    
// 0x8823: push   word ptr [0109]
// 0x8827: mov    ax,0000
// 0x882a: es:    
// 0x882b: mov    [0109],ah
// 0x882f: mov    ax,0006
// 0x8832: int    63
// 0x8834: es:    
// 0x8835: pop    word ptr [0109]
// 0x8839: pop    es
// 0x883a: lodsw
// 0x883b: mov    bx,ax
// 0x883d: jmp    word ptr [bx]

// ================================================
// 0x883f: WORD 'LXPLOT' codep=0x884a wordp=0x884a
// ================================================
// 0x884a: pop    ax
// 0x884b: pop    bx
// 0x884c: mov    bh,al
// 0x884e: push   es
// 0x884f: push   word ptr [84F5] // GRSEG
// 0x8853: pop    es
// 0x8854: es:    
// 0x8855: push   word ptr [0109]
// 0x8859: mov    ax,4000
// 0x885c: es:    
// 0x885d: mov    [0109],ah
// 0x8861: mov    ax,0006
// 0x8864: int    63
// 0x8866: es:    
// 0x8867: pop    word ptr [0109]
// 0x886b: pop    es
// 0x886c: lodsw
// 0x886d: mov    bx,ax
// 0x886f: jmp    word ptr [bx]

// ================================================
// 0x8871: WORD 'W8873' codep=0x224c wordp=0x8873
// ================================================

void W8873() // W8873
{
  Push(Read16(cc_CX)); // CX
  C_ex__2(); // C!_2
  Push(Read16(cc_CX) + 1); // CX 1+
  C_ex__2(); // C!_2
  Push(Read16(cc_BX)); // BX
  C_ex__2(); // C!_2
  Push(Read16(cc_BX) + 1); // BX 1+
  C_ex__2(); // C!_2
  Push(0x000a);
  Push(Read16(cc_AX)); // AX
  Store_2(); // !_2
  GRCALL(); // GRCALL
}


// ================================================
// 0x8893: WORD 'BFILL' codep=0x224c wordp=0x889d params=0 returns=0
// ================================================

void BFILL() // BFILL
{
  W87BD(); // W87BD
  W86FA(); // W86FA
  Push(0x00c7);
  Push2Words("NULL");
  Push(0x00a0);
  W8873(); // W8873
  W87CB(); // W87CB
}


// ================================================
// 0x88b1: WORD 'DARK' codep=0x224c wordp=0x88ba
// ================================================

void DARK() // DARK
{
  W87BD(); // W87BD
  W86FA(); // W86FA
  GetColor(BLACK);
  StoreCOLOR(); // !COLOR
  Push(0x00c7);
  Push2Words("NULL");
  Push(0x00a0);
  W8873(); // W8873
  W87CB(); // W87CB
}


// ================================================
// 0x88d2: WORD 'W88D4' codep=0x224c wordp=0x88d4
// ================================================

void W88D4() // W88D4
{
  Push(8);
  Push(Read16(cc_AX)); // AX
  Store_2(); // !_2
  Push(Read16(cc_CX) + 1); // CX 1+
  C_ex__2(); // C!_2
  Push(Read16(cc_CX)); // CX
  C_ex__2(); // C!_2
  Push(Read16(cc_BX) + 1); // BX 1+
  C_ex__2(); // C!_2
  Push(Read16(cc_BX)); // BX
  C_ex__2(); // C!_2
  GRCALL(); // GRCALL
}


// ================================================
// 0x88f2: WORD 'LLINE' codep=0x224c wordp=0x88fc params=4 returns=0
// ================================================

void LLINE() // LLINE
{
  unsigned short int a;
  W87BD(); // W87BD
  a = Pop(); // >R
  W86FA(); // W86FA
  W88D4(); // W88D4
  Push(a); // R>
  W87CB(); // W87CB
}


// ================================================
// 0x890a: WORD 'FCIRC' codep=0x8914 wordp=0x8914
// ================================================
// 0x8914: pop    bx
// 0x8915: mov    dx,[16BC]
// 0x8919: pop    ax
// 0x891a: pop    cx
// 0x891b: push   di
// 0x891c: mov    di,ax
// 0x891e: mov    ax,0004
// 0x8921: int    63
// 0x8923: pop    di
// 0x8924: lodsw
// 0x8925: mov    bx,ax
// 0x8927: jmp    word ptr [bx]

// ================================================
// 0x8929: WORD 'W892B' codep=0x224c wordp=0x892b params=0 returns=0
// ================================================

void W892B() // W892B
{
  Push(6);
  Push(Read16(cc_DX) + 1); // DX 1+
  C_ex__2(); // C!_2
  Push(5);
  Push(Read16(cc_DX)); // DX
  C_ex__2(); // C!_2
}


// ================================================
// 0x893b: WORD 'W893D' codep=0x224c wordp=0x893d params=2 returns=0
// ================================================

void W893D() // W893D
{
  Push(Pop() * 3); //  3 *
  Push(Read16(cc_DX)); // DX
  C_ex__2(); // C!_2
  Push(Pop() * 6); //  6 *
  Push(Read16(cc_DX) + 1); // DX 1+
  C_ex__2(); // C!_2
}


// ================================================
// 0x8951: WORD 'W8953' codep=0x8953 wordp=0x8953
// ================================================
// 0x8953: pop    bx
// 0x8954: mov    dx,[16BC]
// 0x8958: pop    ax
// 0x8959: pop    cx
// 0x895a: push   di
// 0x895b: mov    di,ax
// 0x895d: mov    ax,0003
// 0x8960: int    63
// 0x8962: pop    di
// 0x8963: lodsw
// 0x8964: mov    bx,ax
// 0x8966: jmp    word ptr [bx]

// ================================================
// 0x8968: WORD '.CIRCLE_1' codep=0x224c wordp=0x8974
// ================================================

void DrawCIRCLE_1() // .CIRCLE_1
{
  W892B(); // W892B
  W8953(); // W8953
}


// ================================================
// 0x897a: WORD 'FILLCIR' codep=0x224c wordp=0x8986
// ================================================

void FILLCIR() // FILLCIR
{
  W892B(); // W892B
  FCIRC(); // FCIRC
}


// ================================================
// 0x898c: WORD '.ELLIPS' codep=0x224c wordp=0x8998
// ================================================

void DrawELLIPS() // .ELLIPS
{
  W893D(); // W893D
  W8953(); // W8953
}


// ================================================
// 0x899e: WORD 'FILLELL' codep=0x224c wordp=0x89aa
// ================================================

void FILLELL() // FILLELL
{
  W893D(); // W893D
  FCIRC(); // FCIRC
}


// ================================================
// 0x89b0: WORD 'W89B2' codep=0x1d29 wordp=0x89b2 params=0 returns=1
// ================================================
// orphan
// 0x89b2: push   es
// 0x89b3: push   word ptr [84F5] // GRSEG
// 0x89b7: pop    es
// 0x89b8: es:    
// 0x89b9: mov    ax,[0108]
// 0x89bd: mov    cx,[5A56] // XORMODE
// 0x89c1: or     cx,cx
// 0x89c3: jnz    89CA
// 0x89c5: and    ax,00FF
// 0x89c8: jmp    89CD
// 0x89ca: and    ax,40FF
// 0x89cd: es:    
// 0x89ce: mov    [0108],ax
// 0x89d2: pop    es
// 0x89d3: mov    ax,0005
// 0x89d6: mov    cx,[596F] // YLINE
// 0x89da: mov    bx,[5958] // XSTART
// 0x89de: mov    dx,[5963] // XEND
// 0x89e2: int    63
// 0x89e4: ret    


// ================================================
// 0x89e5: WORD 'HLINE' codep=0x89ef wordp=0x89ef
// ================================================
// 0x89ef: call   89B2
// 0x89f2: lodsw
// 0x89f3: mov    bx,ax
// 0x89f5: jmp    word ptr [bx]

// ================================================
// 0x89f7: WORD 'W89F9' codep=0x1d29 wordp=0x89f9
// ================================================
// 0x89f9: db 0x00 0x04 0x05 0x05 '    '

// ================================================
// 0x89fd: WORD 'W89FF' codep=0x224c wordp=0x89ff
// ================================================
// orphan

void W89FF() // W89FF
{
  Push(Read16(cc_BX)); // BX
  Store_2(); // !_2
  Push(2);
  Push(Read16(cc_AX)); // AX
  Store_2(); // !_2
  Push(Read16(pp_XBLT)); // XBLT @
  Push(Read16(cc_CX)); // CX
  C_ex__2(); // C!_2
  Push(Read16(pp_YBLT)); // YBLT @
  Push(Read16(cc_CX) + 1); // CX 1+
  C_ex__2(); // C!_2
  GRCALL(); // GRCALL
  Push(Read16(Read16(pp_FONT_n_) + pp_W89F9)&0xFF); // FONT# @ W89F9 + C@
  Push(pp_XBLT); // XBLT
  _plus__ex__2(); // +!_2
}


// ================================================
// 0x8a2d: WORD 'W8A2F' codep=0x224c wordp=0x8a2f
// ================================================

void W8A2F() // W8A2F
{
  unsigned short int i, imax;
  _2DUP(); // 2DUP
  _gt_UPPERCASE(); // >UPPERCASE

  i = 0;
  imax = Pop();
  do // (DO)
  {
    Push(Read16(regsp)); // DUP
    Push(Read16(Pop())&0xFF); //  C@
    W89FF(); // W89FF
    Push(Pop() + 1); //  1+
    i++;
  } while(i<imax); // (LOOP)

  Pop(); // DROP
}


// ================================================
// 0x8a47: WORD 'W8A49' codep=0x224c wordp=0x8a49
// ================================================

void W8A49() // W8A49
{
  Push(Read16(regsp)); // DUP
  Push(pp_FONT_n_); // FONT#
  Store_2(); // !_2
  Push(0x0018);
  Push(Read16(cc_AX)); // AX
  Store_2(); // !_2
  Push(Pop() - 1); //  1-
  Push(Read16(cc_BX)); // BX
  Store_2(); // !_2
  GRCALL(); // GRCALL
  Push(0x89ff); // 'W89FF'
  Push(0x066d); // 'EMIT'
  EXECUTES(); // EXECUTES
  Push(0x8a2f); // 'W8A2F'
  Push(0x06e8); // 'TYPE'
  EXECUTES(); // EXECUTES
}


// ================================================
// 0x8a75: WORD '>1FONT' codep=0x224c wordp=0x8a80
// ================================================

void _gt_1FONT() // >1FONT
{
  Push(1);
  W8A49(); // W8A49
}


// ================================================
// 0x8a86: WORD '>2FONT' codep=0x224c wordp=0x8a91
// ================================================

void _gt_2FONT() // >2FONT
{
  Push(2);
  W8A49(); // W8A49
}


// ================================================
// 0x8a97: WORD '>3FONT' codep=0x224c wordp=0x8aa2
// ================================================

void _gt_3FONT() // >3FONT
{
  Push(3);
  W8A49(); // W8A49
}


// ================================================
// 0x8aa8: WORD '>0FONT_2' codep=0x224c wordp=0x8ab3 params=0 returns=0
// ================================================

void _gt_0FONT_2() // >0FONT_2
{
  Push(pp_FONT_n_); // FONT#
  _099(); // 099
  _gt_0FONT_1(); // >0FONT_1
}


// ================================================
// 0x8abb: WORD 'LDPAL' codep=0x224c wordp=0x8ac5
// ================================================

void LDPAL() // LDPAL
{
  PAD_gt_SEG(); // PAD>SEG
  Push(0x002c);
  FILE_st_(); // FILE<
  Push(Pop() * 0x0012); //  0x0012 *
  PAD_v_16(); // PAD|16
  Push(Pop() + Pop()); // +
  Push(pp_COLORT); // COLORT
  Push(0x0010);
  CMOVE_2(); // CMOVE_2
}


// ================================================
// 0x8ae1: WORD 'W8AE3' codep=0x224c wordp=0x8ae3 params=1 returns=0
// ================================================

void W8AE3() // W8AE3
{
  Push(Read16(pp_REMSEG)); // REMSEG @
  Push(0x010a);
  L_ex_(); // L!
}


// ================================================
// 0x8aef: WORD ':8>' codep=0x224c wordp=0x8af7 params=0 returns=0
// ================================================

void _c_8_gt_() // :8>
{
  Push(4);
  W8AE3(); // W8AE3
}


// ================================================
// 0x8afd: WORD ')8>' codep=0x224c wordp=0x8b05 params=0 returns=0
// ================================================

void _rc_8_gt_() // )8>
{
  Push(2);
  W8AE3(); // W8AE3
}


// ================================================
// 0x8b0b: WORD '2<8>' codep=0x224c wordp=0x8b14 params=0 returns=0
// ================================================

void _2_st_8_gt_() // 2<8>
{
  Push(2);
  W8AE3(); // W8AE3
}


// ================================================
// 0x8b1a: WORD '<8>' codep=0x224c wordp=0x8b22 params=0 returns=0
// ================================================

void _st_8_gt_() // <8>
{
  Push(8);
  W8AE3(); // W8AE3
}


// ================================================
// 0x8b28: WORD 'HERC' codep=0x224c wordp=0x8b31 params=0 returns=0
// ================================================

void HERC() // HERC
{
  Push(Read16(cc_TRUE)); // TRUE
  Push(Read16(pp_REMSEG)); // REMSEG @
  Push(0x0104);
  LC_ex_(); // LC!
  _st_8_gt_(); // <8>
}


// ================================================
// 0x8b41: WORD '#VECTOR' codep=0x2214 wordp=0x8b4d
// ================================================
// 0x8b4d: dw 0x0006

// ================================================
// 0x8b4f: WORD 'TREJECT' codep=0x1d29 wordp=0x8b5b
// ================================================
// 0x8b5b: db 0x9a 0x41 0x00 0x37 0x7f 0xc3 ' A 7 '

// ================================================
// 0x8b61: WORD 'CLIPPER' codep=0x8b6d wordp=0x8b6d
// ================================================
// 0x8b6d: call   7F37:029D
// 0x8b72: lodsw
// 0x8b73: mov    bx,ax
// 0x8b75: jmp    word ptr [bx]

// ================================================
// 0x8b77: WORD 'SCANPOL' codep=0x8b83 wordp=0x8b83
// ================================================
// 0x8b83: call   7F37:0535
// 0x8b88: lodsw
// 0x8b89: mov    bx,ax
// 0x8b8b: jmp    word ptr [bx]

// ================================================
// 0x8b8d: WORD '>PLANES' codep=0x8b99 wordp=0x8b99
// ================================================
// 0x8b99: call   7F37:0837
// 0x8b9e: lodsw
// 0x8b9f: mov    bx,ax
// 0x8ba1: jmp    word ptr [bx]

// ================================================
// 0x8ba3: WORD 'LFILL' codep=0x8bad wordp=0x8bad
// ================================================
// 0x8bad: call   7F37:0413
// 0x8bb2: lodsw
// 0x8bb3: mov    bx,ax
// 0x8bb5: jmp    word ptr [bx]

// ================================================
// 0x8bb7: WORD 'BLT' codep=0x8bbf wordp=0x8bbf
// ================================================
// 0x8bbf: call   7F37:070A
// 0x8bc4: lodsw
// 0x8bc5: mov    bx,ax
// 0x8bc7: jmp    word ptr [bx]

// ================================================
// 0x8bc9: WORD 'LFILLPO' codep=0x224c wordp=0x8bd5
// ================================================

void LFILLPO() // LFILLPO
{
  unsigned short int a;
  W87BD(); // W87BD
  a = Pop(); // >R
  W86FA(); // W86FA
  LFILL(); // LFILL
  Push(a); // R>
  W87CB(); // W87CB
}


// ================================================
// 0x8be3: WORD 'W8BE5' codep=0x224c wordp=0x8be5
// ================================================

void W8BE5() // W8BE5
{
  Push(Read16(cc_BX)); // BX
  C_ex__2(); // C!_2
  Push(Read16(cc_CX) + 1); // CX 1+
  C_ex__2(); // C!_2
  Push(Read16(cc_CX)); // CX
  C_ex__2(); // C!_2
  Push(Read16(cc_BX) + 1); // BX 1+
  C_ex__2(); // C!_2
  Push(0x0016);
  Push(Read16(cc_AX)); // AX
  Store_2(); // !_2
  GRCALL(); // GRCALL
}


// ================================================
// 0x8c05: WORD 'SETCLIP' codep=0x224c wordp=0x8c11
// ================================================

void SETCLIP() // SETCLIP
{
  _2OVER(); // 2OVER
  _2OVER(); // 2OVER
  W8BE5(); // W8BE5
  Push(Read16(regsp)); // DUP
  Push(pp_ILEFT); // ILEFT
  Store_2(); // !_2
  Push(pp_CLIP_dash_TA + 0x0012); // CLIP-TA 0x0012 +
  Store_2(); // !_2
  Push(Read16(regsp)); // DUP
  Push(pp_IBELOW); // IBELOW
  Store_2(); // !_2
  Push(pp_CLIP_dash_TA + 0x000d); // CLIP-TA 0x000d +
  Store_2(); // !_2
  Push(Read16(regsp)); // DUP
  Push(pp_IRIGHT); // IRIGHT
  Store_2(); // !_2
  Push(pp_CLIP_dash_TA + 8); // CLIP-TA 8 +
  Store_2(); // !_2
  Push(Read16(regsp)); // DUP
  Push(pp_IABOVE); // IABOVE
  Store_2(); // !_2
  Push(pp_CLIP_dash_TA + 3); // CLIP-TA 3 +
  Store_2(); // !_2
}


// ================================================
// 0x8c55: WORD 'VDCLIPS' codep=0x224c wordp=0x8c61
// ================================================

void VDCLIPS() // VDCLIPS
{
  Push(0x00bf);
  Push(0x004b);
  Push(0x0048);
  Push(4);
  SETCLIP(); // SETCLIP
}


// ================================================
// 0x8c73: WORD 'VCLIPSE' codep=0x224c wordp=0x8c7f
// ================================================

void VCLIPSE() // VCLIPSE
{
  Push(0x0079);
  Push(0x0047);
  Push(0);
  Push(0);
  SETCLIP(); // SETCLIP
}


// ================================================
// 0x8c8f: WORD 'DCLIPSE' codep=0x224c wordp=0x8c9b
// ================================================

void DCLIPSE() // DCLIPSE
{
  Push(0x00c7);
  Push(0x009f);
  Push(0);
  Push(0);
  SETCLIP(); // SETCLIP
}


// ================================================
// 0x8cab: WORD 'W8CAD' codep=0x1d29 wordp=0x8cad
// ================================================
// 0x8cad: db 0x5b 0x8b 0x6d 0x8b 0x83 0x8b 0x99 0x8b 0xad 0x8b 0xbf 0x8b '[ m         '

// ================================================
// 0x8cb9: WORD 'W8CBB' codep=0x224c wordp=0x8cbb params=0 returns=0
// ================================================

void W8CBB() // W8CBB
{
  unsigned short int i, imax;

  i = 0;
  imax = Read16(cc__n_VECTOR) * 2; // #VECTOR 2*
  do // (DO)
  {
    Push(Read16(pp_FONTSEG)); // FONTSEG @
    Push(Read16(pp_W8CAD + i) + 3); // W8CAD I + @ 3 +
    Store_2(); // !_2
    Push(2);
    int step = Pop();
    i += step;
    if (((step>=0) && (i>=imax)) || ((step<0) && (i<=imax))) break;
  } while(1); // (+LOOP)

}


// ================================================
// 0x8cdd: WORD 'W8CDF' codep=0x224c wordp=0x8cdf params=0 returns=0
// ================================================

void W8CDF() // W8CDF
{
  unsigned short int i, imax;

  i = 0;
  imax = Read16(cc__n_VECTOR) * 2; // #VECTOR 2*
  do // (DO)
  {
    Push(Read16(pp_FONTSEG)); // FONTSEG @
    Push(i); // I
    L_at_(); // L@
    Push(Read16(pp_W8CAD + i) + 1); // W8CAD I + @ 1+
    Store_2(); // !_2
    Push(2);
    int step = Pop();
    i += step;
    if (((step>=0) && (i>=imax)) || ((step<0) && (i<=imax))) break;
  } while(1); // (+LOOP)

}


// ================================================
// 0x8d03: WORD 'W8D05' codep=0x224c wordp=0x8d05
// ================================================

void W8D05() // W8D05
{
  Push(Read16(pp_FONTSEG)); // FONTSEG @
  Push(0x008f);
  FILE_st_(); // FILE<
  W8CBB(); // W8CBB
  W8CDF(); // W8CDF
}


// ================================================
// 0x8d15: WORD '>MAINVI' codep=0x224c wordp=0x8d21
// ================================================

void _gt_MAINVI() // >MAINVI
{
  Push(Read16(pp_HBUF_dash_SE)); // HBUF-SE @
  Push(pp_BUF_dash_SEG); // BUF-SEG
  Store_2(); // !_2
  Push(Read16(pp_BUF_dash_SEG)); // BUF-SEG @
  Push(Read16(pp_GRSEG)); // GRSEG @
  Push(0x02a4);
  L_ex_(); // L!
  VCLIPSE(); // VCLIPSE
}


// ================================================
// 0x8d3b: WORD '>DISPLA' codep=0x224c wordp=0x8d47
// ================================================

void _gt_DISPLA() // >DISPLA
{
  DCLIPSE(); // DCLIPSE
  Push(Read16(pp_DBUF_dash_SE)); // DBUF-SE @
  Push(Read16(regsp)); // DUP
  Push(pp_BUF_dash_SEG); // BUF-SEG
  Store_2(); // !_2
  Push(Read16(pp_GRSEG)); // GRSEG @
  Push(0x02a4);
  L_ex_(); // L!
}


// ================================================
// 0x8d5f: WORD '>HIDDEN' codep=0x224c wordp=0x8d6b
// ================================================

void _gt_HIDDEN() // >HIDDEN
{
  DCLIPSE(); // DCLIPSE
  Push(Read16(pp_HBUF_dash_SE)); // HBUF-SE @
  Push(Read16(regsp)); // DUP
  Push(pp_BUF_dash_SEG); // BUF-SEG
  Store_2(); // !_2
  Push(Read16(pp_GRSEG)); // GRSEG @
  Push(0x02a4);
  L_ex_(); // L!
}


// ================================================
// 0x8d83: WORD 'R2OVL' codep=0x2214 wordp=0x8d8d
// ================================================
// 0x8d8d: dw 0x8d90
// 0x8d8f: db 0x00 0x72 0x32 0x2e 0x6f 0x76 0x6c 0x00 ' r2.ovl '

// ================================================
// 0x8d97: WORD 'W8D99' codep=0x8d99 wordp=0x8d99
// ================================================
// 0x8d99: call   953E:0100
// 0x8d9e: lodsw
// 0x8d9f: mov    bx,ax
// 0x8da1: jmp    word ptr [bx]

// ================================================
// 0x8da3: WORD 'W8DA5' codep=0x224c wordp=0x8da5
// ================================================

void W8DA5() // W8DA5
{
  Push(Read16(cc_R2OVL)); // R2OVL
  Push(Read16(pp_REMSEG) + 0x0010); // REMSEG @ 0x0010 +
  _ro_LDS_rc_(); // (LDS)
  Push(Read16(pp_REMSEG)); // REMSEG @
  Push(0x8d99); // 'W8D99'
  Push(Pop() + 3); //  3+
  Store_2(); // !_2
  W8D99(); // W8D99
}


// ================================================
// 0x8dc3: WORD 'W8DC5' codep=0x224c wordp=0x8dc5
// ================================================

void W8DC5() // W8DC5
{
  Push(0xabcd);
  Push(Read16(cc_AX)); // AX
  Store_2(); // !_2
  GRCALL(); // GRCALL
}


// ================================================
// 0x8dd1: WORD 'SRC' codep=0x1d29 wordp=0x8dd9
// ================================================
// 0x8dd9: db 0x20 0x33 ' 3'

// ================================================
// 0x8ddb: WORD 'DST' codep=0x1d29 wordp=0x8de3
// ================================================
// 0x8de3: db 0x00 0x83 '  '

// ================================================
// 0x8de5: WORD 'SEGME' codep=0x1d29 wordp=0x8def
// ================================================
// 0x8def: db 0x20 0x8d '  '

// ================================================
// 0x8df1: WORD 'PLZ' codep=0x1d29 wordp=0x8df9
// ================================================
// 0x8df9: db 0x22 0x5d '"]'

// ================================================
// 0x8dfb: WORD 'XPCOL' codep=0x1d29 wordp=0x8e05
// ================================================
// 0x8e05: db 0x00 0x00 '  '

// ================================================
// 0x8e07: WORD '.RAW' codep=0x224c wordp=0x8e10
// ================================================

void DrawRAW() // .RAW
{
  Push(Read16(pp_SKIPPED)); // SKIPPED @
  if (Pop() != 0)
  {
    Push(pp_SKIPPED); // SKIPPED
    _099(); // 099
    Pop(); Pop(); // 2DROP
    Push(pp_PIC_n_); // PIC#
    _099(); // 099
    return;
  }
  Push(1);
  Push(Read16(cc_AX)); // AX
  Store_2(); // !_2
  Push(pp_BMAP); // BMAP
  Push(Read16(cc_BX)); // BX
  Store_2(); // !_2
  Push(Read16(cc_CX) + 1); // CX 1+
  C_ex__2(); // C!_2
  Push(Pop() + 0x0028); //  0x0028 +
  Push(Read16(cc_CX)); // CX
  C_ex__2(); // C!_2
  GRCALL(); // GRCALL
}


// ================================================
// 0x8e46: WORD 'DISPLAY' codep=0x8e52 wordp=0x8e52
// ================================================
// 0x8e52: lodsw
// 0x8e53: mov    bx,ax
// 0x8e55: jmp    word ptr [bx]
// 0x8e57: db 0x59 0x8e 0x5a 0x1e 0x33 0xc0 0x8e 0xd8 0xbb 0x10 0x04 0x89 0x17 0x1f 0xad 0x8b 0xd8 0xff 0x27 'Y Z 3             ''

// ================================================
// 0x8e6a: WORD '>ALPHA' codep=0x224c wordp=0x8e75
// ================================================

void _gt_ALPHA() // >ALPHA
{
  Push(0x0013);
  Push(Read16(cc_AX)); // AX
  Store_2(); // !_2
  GRCALL(); // GRCALL
  _gt_0FONT_2(); // >0FONT_2
}


// ================================================
// 0x8e83: WORD '>A' codep=0x224c wordp=0x8e8a
// ================================================

void _gt_A() // >A
{
  _gt_ALPHA(); // >ALPHA
}


// ================================================
// 0x8e8e: WORD '>LORES' codep=0x224c wordp=0x8e99
// ================================================

void _gt_LORES() // >LORES
{
  Push(0x0012);
  Push(Read16(cc_AX)); // AX
  Store_2(); // !_2
  GRCALL(); // GRCALL
  _2PIX(); // 2PIX
  SETDBUF(); // SETDBUF
  IsCGA(); // ?CGA
  if (Pop() == 0) return;
  Push(0);
  Push(Read16(cc_BX)); // BX
  Store_2(); // !_2
  Push(0x000e);
  Push(Read16(cc_AX)); // AX
  Store_2(); // !_2
  GRCALL(); // GRCALL
}


// ================================================
// 0x8ebf: WORD 'W8EC1' codep=0x224c wordp=0x8ec1
// ================================================

void W8EC1() // W8EC1
{
  Push(Read16(cc_AX)); // AX
  _099(); // 099
  Push(0x00c7);
  Push(Read16(cc_BX) + 1); // BX 1+
  C_ex__2(); // C!_2
  Push(0);
  Push(Read16(cc_BX)); // BX
  C_ex__2(); // C!_2
  Push(0);
  Push(Read16(cc_CX) + 1); // CX 1+
  C_ex__2(); // C!_2
  Push(0x009f);
  Push(Read16(cc_CX)); // CX
  C_ex__2(); // C!_2
  Push(0x00c7);
  Push(Read16(cc_DX) + 1); // DX 1+
  C_ex__2(); // C!_2
  Push(0);
  Push(Read16(cc_DX)); // DX
  C_ex__2(); // C!_2
  GRCALL(); // GRCALL
}


// ================================================
// 0x8ef9: WORD 'W8EFB' codep=0x224c wordp=0x8efb params=2 returns=0
// ================================================

void W8EFB() // W8EFB
{
  Push(Read16(pp_GRSEG)); // GRSEG @
  Push(0x010e);
  L_ex_(); // L!
  Push(Read16(pp_GRSEG)); // GRSEG @
  Push(0x010c);
  L_ex_(); // L!
}


// ================================================
// 0x8f11: WORD 'D>H' codep=0x224c wordp=0x8f19 params=0 returns=0
// ================================================

void D_gt_H() // D>H
{
  Push(Read16(pp_DBUF_dash_SE)); // DBUF-SE @
  Push(Read16(pp_HBUF_dash_SE)); // HBUF-SE @
  W8EFB(); // W8EFB
}


// ================================================
// 0x8f25: WORD 'H>D' codep=0x224c wordp=0x8f2d params=0 returns=0
// ================================================

void H_gt_D() // H>D
{
  Push(Read16(pp_HBUF_dash_SE)); // HBUF-SE @
  Push(Read16(pp_DBUF_dash_SE)); // DBUF-SE @
  W8EFB(); // W8EFB
}


// ================================================
// 0x8f39: WORD 'SAVE-SC' codep=0x224c wordp=0x8f45
// ================================================

void SAVE_dash_SC() // SAVE-SC
{
  D_gt_H(); // D>H
  W8EC1(); // W8EC1
}


// ================================================
// 0x8f4b: WORD 'SCR-RES' codep=0x224c wordp=0x8f57
// ================================================

void SCR_dash_RES() // SCR-RES
{
  H_gt_D(); // H>D
  W8EC1(); // W8EC1
}


// ================================================
// 0x8f5d: WORD 'V>DISPL' codep=0x224c wordp=0x8f69
// ================================================

void V_gt_DISPL() // V>DISPL
{
  H_gt_D(); // H>D
  Push(Read16(cc_AX)); // AX
  _099(); // 099
  Push(0x0077);
  Push(Read16(cc_BX) + 1); // BX 1+
  C_ex__2(); // C!_2
  Push(0);
  Push(Read16(cc_BX)); // BX
  C_ex__2(); // C!_2
  Push(0);
  Push(Read16(cc_CX) + 1); // CX 1+
  C_ex__2(); // C!_2
  Push(0x0047);
  Push(Read16(cc_CX)); // CX
  C_ex__2(); // C!_2
  Push(0x00bf);
  Push(Read16(cc_DX) + 1); // DX 1+
  C_ex__2(); // C!_2
  Push(4);
  Push(Read16(cc_DX)); // DX
  C_ex__2(); // C!_2
  GRCALL(); // GRCALL
}


// ================================================
// 0x8fa3: WORD 'W8FA5' codep=0x224c wordp=0x8fa5
// ================================================

void W8FA5() // W8FA5
{
  Push(Read16(cc_AX)); // AX
  _099(); // 099
  Push(Read16(cc_DX) + 1); // DX 1+
  C_ex__2(); // C!_2
  Push(Read16(cc_DX)); // DX
  C_ex__2(); // C!_2
  Push(Read16(cc_CX) + 1); // CX 1+
  C_ex__2(); // C!_2
  Push(Read16(cc_CX)); // CX
  C_ex__2(); // C!_2
  Push(Read16(cc_BX) + 1); // BX 1+
  C_ex__2(); // C!_2
  Push(Read16(cc_BX)); // BX
  C_ex__2(); // C!_2
  GRCALL(); // GRCALL
}


// ================================================
// 0x8fcb: WORD 'W8FCD' codep=0x224c wordp=0x8fcd params=0 returns=1
// ================================================

void W8FCD() // W8FCD
{
  unsigned short int i, imax;

  i = 0;
  imax = 6;
  do // (DO)
  {
    Push(6);
    PICK(); // PICK
    i++;
  } while(i<imax); // (LOOP)

}


// ================================================
// 0x8fdd: WORD 'LCOPYBL' codep=0x224c wordp=0x8fe9
// ================================================

void LCOPYBL() // LCOPYBL
{
  Push(Read16(regsp)); // DUP
  Push(6);
  PICK(); // PICK
  _st_(); // <
  if (Pop() != 0)
  {
    SAVE_dash_SC(); // SAVE-SC
    D_gt_H(); // D>H
    W8FCD(); // W8FCD
    W8FA5(); // W8FA5
    H_gt_D(); // H>D
    Pop(); // DROP
    Push(4);
    PICK(); // PICK
  } else
  {
    Push(Read16(pp_BUF_dash_SEG)); // BUF-SEG @
    Push(Read16(regsp)); // DUP
    W8EFB(); // W8EFB
  }
  W8FA5(); // W8FA5
  _gt_DISPLA(); // >DISPLA
}


// ================================================
// 0x9017: WORD '(BYE)' codep=0x224c wordp=0x9021
// ================================================

void _ro_BYE_rc_() // (BYE)
{
  _gt_ALPHA(); // >ALPHA
  Exec("PAGE"); // call of word 0x274c '(PAGE)'
  BEEPOFF(); // BEEPOFF
  IsHERCUL(); // ?HERCUL
  if (Pop() != 0)
  {
    _gt_DISPLA(); // >DISPLA
    DARK(); // DARK
    Exec("PAGE"); // call of word 0x274c '(PAGE)'
  }
  W8DC5(); // W8DC5
  BYE_1(); // BYE_1
}


// ================================================
// 0x9039: WORD 'BYE_2' codep=0x224c wordp=0x9041
// ================================================

void BYE_2() // BYE_2
{
  Push(pp__i_BYE); // 'BYE
  GetEXECUTE(); // @EXECUTE
}


// ================================================
// 0x9047: WORD 'W9049' codep=0x224c wordp=0x9049 params=0 returns=0
// ================================================

void W9049() // W9049
{
  unsigned short int i, imax;

  i = 0;
  imax = 0x003c;
  do // (DO)
  {
    Push(Read16(i + pp_CMAP)&0xFF); // I CMAP + C@
    I_gt_C(); // I>C
    Push(i + pp_CMAP); // I CMAP +
    C_ex__2(); // C!_2
    i++;
  } while(i<imax); // (LOOP)

}


// ================================================
// 0x9069: WORD 'SET-COL' codep=0x224c wordp=0x9075
// ================================================

void SET_dash_COL() // SET-COL
{
  GetMODE(); // @MODE
  Push(Pop()==8?1:0); //  8 =
  if (Pop() != 0)
  {
    Push(0);
  } else
  {
    Push(5);
  }
  Push(Pop() + (Read16(pp__n_CLRMAP) - 1)); //  #CLRMAP @ 1- +
  Push(0x0074);
  SWAP(); // SWAP
  GetRECORD(); // @RECORD
  Push(pp_CMAP); // CMAP
  Push(0x0040);
  CMOVE_2(); // CMOVE_2
  W9049(); // W9049
}


// ================================================
// 0x90a3: WORD 'COLORMA_1' codep=0x90af wordp=0x90af
// ================================================
// 0x90af: pop    bx
// 0x90b0: or     bx,bx
// 0x90b2: jns    90B9
// 0x90b4: mov    bx,0000
// 0x90b7: jmp    90BE
// 0x90b9: shr    bx,1
// 0x90bb: and    bx,38
// 0x90be: add    bx,699A
// 0x90c2: mov    [5A9D],bx // TILE-PT
// 0x90c6: xor    ax,ax
// 0x90c8: mov    al,[bx]
// 0x90ca: mov    [5747],ax // COLOR
// 0x90ce: add    bx,02
// 0x90d1: mov    al,[bx]
// 0x90d3: mov    [5754],ax // DCOLOR
// 0x90d7: lodsw
// 0x90d8: mov    bx,ax
// 0x90da: jmp    word ptr [bx]

// ================================================
// 0x90dc: WORD 'COLORMA_2' codep=0x224c wordp=0x90e8
// ================================================

void COLORMA_2() // COLORMA_2
{
  COLORMA_1(); // COLORMA_1
  Push(Read16(pp_COLOR)); // COLOR @
  StoreCOLOR(); // !COLOR
}


// ================================================
// 0x90f2: WORD 'CELLCOL' codep=0x224c wordp=0x90fe
// ================================================

void CELLCOL() // CELLCOL
{
  ACELLAD(); // ACELLAD
  A_at_(); // A@
  COLORMA_2(); // COLORMA_2
}


// ================================================
// 0x9106: WORD '!XYSEED' codep=0x224c wordp=0x9112 params=2 returns=0
// ================================================

void StoreXYSEED() // !XYSEED
{
  Push(pp_SEED_3); // SEED_3
  Store_2(); // !_2
  FRND(); // FRND
  SWAP(); // SWAP
  Push(pp_SEED_3); // SEED_3
  Store_2(); // !_2
  FRND(); // FRND
  Push(Pop() + Read16(pp_GLOBALS)); //  GLOBALS @ XOR
  Push(Pop() ^ Pop()); // XOR
  Push(pp_SEED_3); // SEED_3
  Store_2(); // !_2
}


// ================================================
// 0x912e: WORD 'SQLPLOT' codep=0x224c wordp=0x913a params=2 returns=0
// ================================================

void SQLPLOT() // SQLPLOT
{
  unsigned short int a, b;
  Push(Read16(regsp)); // DUP
  Push(Pop() - 1); //  1-
  a = Pop(); // >R
  OVER(); // OVER
  b = Pop(); // >R
  LPLOT(); // LPLOT
  Push(b); // R>
  Push(a); // R>
  LPLOT(); // LPLOT
}


// ================================================
// 0x914e: WORD 'BUFFERX' codep=0x915a wordp=0x915a
// ================================================
// 0x915a: pop    cx
// 0x915b: pop    ax
// 0x915c: pop    dx
// 0x915d: pop    bx
// 0x915e: sub    dx,[4EC2] // YLL
// 0x9162: inc    dx
// 0x9163: or     cx,cx
// 0x9165: jz     9169
// 0x9167: shl    dx,cl
// 0x9169: add    dx,[5B3B] // YLLDEST
// 0x916d: dec    dx
// 0x916e: sub    bx,[4EB8] // XLL
// 0x9172: mov    cx,ax
// 0x9174: or     cx,cx
// 0x9176: jz     917A
// 0x9178: shl    bx,cl
// 0x917a: add    bx,[5B2D] // XLLDEST
// 0x917e: push   bx
// 0x917f: push   dx
// 0x9180: lodsw
// 0x9181: mov    bx,ax
// 0x9183: jmp    word ptr [bx]

// ================================================
// 0x9185: WORD 'W9187' codep=0x224c wordp=0x9187
// ================================================

void W9187() // W9187
{
  Push(pp__i__dot_CELL); // '.CELL
  GetEXECUTE(); // @EXECUTE
}


// ================================================
// 0x918d: WORD '.1X2CEL' codep=0x224c wordp=0x9199
// ================================================

void Draw1X2CEL() // .1X2CEL
{
  _2DUP(); // 2DUP
  CELLCOL(); // CELLCOL
  Push(0);
  Push(1);
  BUFFERX(); // BUFFERX
  SQLPLOT(); // SQLPLOT
}


// ================================================
// 0x91a7: WORD 'TILEFIL' codep=0x224c wordp=0x91b3
// ================================================

void TILEFIL() // TILEFIL
{
  unsigned short int a, b;
  W87BD(); // W87BD
  a = Pop(); // >R
  W86FA(); // W86FA
  Push(Read16(Read16(pp_TILE_dash_PT))&0xFF); // TILE-PT @ C@
  StoreCOLOR(); // !COLOR
  b = Pop(); // >R
  OVER(); // OVER
  SWAP(); // SWAP
  _dash_(); // -
  Push(Read16(cc_CX) + 1); // CX 1+
  C_ex__2(); // C!_2
  Push(Read16(cc_BX) + 1); // BX 1+
  C_ex__2(); // C!_2
  Push(b); // R>
  OVER(); // OVER
  Push(Pop() + Pop()); // +
  Push(Read16(cc_CX)); // CX
  C_ex__2(); // C!_2
  Push(Read16(cc_BX)); // BX
  C_ex__2(); // C!_2
  Push(0x000a);
  Push(Read16(cc_AX)); // AX
  Store_2(); // !_2
  GRCALL(); // GRCALL
  Push(a); // R>
  W87CB(); // W87CB
}


// ================================================
// 0x91f3: WORD '.2X2CEL' codep=0x224c wordp=0x91ff
// ================================================

void Draw2X2CEL() // .2X2CEL
{
  _2DUP(); // 2DUP
  CELLCOL(); // CELLCOL
  Push(1);
  Push(1);
  BUFFERX(); // BUFFERX
  _2DUP(); // 2DUP
  SQLPLOT(); // SQLPLOT
  SWAP(); // SWAP
  Push(Pop() + 1); //  1+
  SWAP(); // SWAP
  SQLPLOT(); // SQLPLOT
}


// ================================================
// 0x9217: WORD '.4X4CEL' codep=0x224c wordp=0x9223
// ================================================

void Draw4X4CEL() // .4X4CEL
{
  _2DUP(); // 2DUP
  CELLCOL(); // CELLCOL
  Push(2);
  Push(2);
  BUFFERX(); // BUFFERX
  Push(4);
  Push(4);
  TILEFIL(); // TILEFIL
}


// ================================================
// 0x9235: WORD '.8X8CEL' codep=0x224c wordp=0x9241
// ================================================

void Draw8X8CEL() // .8X8CEL
{
  _2DUP(); // 2DUP
  CELLCOL(); // CELLCOL
  Push(3);
  Push(3);
  BUFFERX(); // BUFFERX
  Push(8);
  Push(8);
  TILEFIL(); // TILEFIL
}


// ================================================
// 0x9253: WORD '.REGION' codep=0x224c wordp=0x925f
// ================================================

void DrawREGION() // .REGION
{
  unsigned short int i, imax, j, jmax;

  i = Read16(pp_YLL); // YLL @
  imax = Read16(pp_YUR) + 1; // YUR @ 1+
  do // (DO)
  {

    j = Read16(pp_XLL); // XLL @
    jmax = Read16(pp_XUR) + 1; // XUR @ 1+
    do // (DO)
    {
      Push(j); // I
      Push(i); // J
      W9187(); // W9187
      j++;
    } while(j<jmax); // (LOOP)

    i++;
  } while(i<imax); // (LOOP)

}


// ================================================
// 0x9287: WORD '?INVIS' codep=0x9292 wordp=0x9292
// ================================================
// 0x9292: sub    ax,ax
// 0x9294: pop    dx
// 0x9295: cmp    dx,[5CCB] // BVIS
// 0x9299: jns    929E
// 0x929b: or     ax,0004
// 0x929e: cmp    dx,[5CB5] // TVIS
// 0x92a2: jle    92A7
// 0x92a4: or     ax,0008
// 0x92a7: pop    dx
// 0x92a8: cmp    dx,[5CD6] // LVIS
// 0x92ac: jns    92B1
// 0x92ae: or     ax,0001
// 0x92b1: cmp    dx,[5CC0] // RVIS
// 0x92b5: jle    92BA
// 0x92b7: or     ax,0002
// 0x92ba: or     ax,ax
// 0x92bc: jnz    92C1
// 0x92be: inc    ax
// 0x92bf: jmp    92C3
// 0x92c1: xor    ax,ax
// 0x92c3: push   ax
// 0x92c4: lodsw
// 0x92c5: mov    bx,ax
// 0x92c7: jmp    word ptr [bx]

// ================================================
// 0x92c9: WORD 'WLD>SCR' codep=0x92d5 wordp=0x92d5
// ================================================
// 0x92d5: pop    ax
// 0x92d6: sub    ax,[5CCB] // BVIS
// 0x92da: mov    cx,[6432] // YWLD:YP
// 0x92de: imul   cx
// 0x92e0: mov    cx,[6434] // YWLD:YP
// 0x92e4: idiv   cx
// 0x92e6: add    ax,[5B3B] // YLLDEST
// 0x92ea: mov    [49B4],ax // ZZZ
// 0x92ee: pop    ax
// 0x92ef: sub    ax,[5CD6] // LVIS
// 0x92f3: mov    cx,[6422] // XWLD:XP
// 0x92f7: imul   cx
// 0x92f9: mov    cx,[6424] // XWLD:XP
// 0x92fd: idiv   cx
// 0x92ff: add    ax,[5B2D] // XLLDEST
// 0x9303: push   ax
// 0x9304: push   word ptr [49B4] // ZZZ
// 0x9308: lodsw
// 0x9309: mov    bx,ax
// 0x930b: jmp    word ptr [bx]

// ================================================
// 0x930d: WORD 'SCR>BLT' codep=0x9319 wordp=0x9319
// ================================================
// 0x9319: pop    ax
// 0x931a: add    ax,0007
// 0x931d: sub    ax,[5C1E] // W5C1E
// 0x9321: pop    cx
// 0x9322: sub    cx,[5C1E] // W5C1E
// 0x9326: push   cx
// 0x9327: push   ax
// 0x9328: lodsw
// 0x9329: mov    bx,ax
// 0x932b: jmp    word ptr [bx]

// ================================================
// 0x932d: WORD '!VISWIN' codep=0x224c wordp=0x9339 params=4 returns=0
// ================================================

void StoreVISWIN() // !VISWIN
{
  Push(pp_TVIS); // TVIS
  Store_2(); // !_2
  Push(pp_RVIS); // RVIS
  Store_2(); // !_2
  Push(pp_BVIS); // BVIS
  Store_2(); // !_2
  Push(pp_LVIS); // LVIS
  Store_2(); // !_2
}


// ================================================
// 0x934b: WORD 'W934D' codep=0x224c wordp=0x934d params=1 returns=0
// ================================================

void W934D() // W934D
{
  Push(Pop() * 0x0043 + Read16(pp_ICON_h_)); //  0x0043 * ICON^ @ +
  Push(0);
  Push2Words("NULL");
  ReadArray(ICONIMA); // ICONIMA
  Push(0x0043 >> 4); // 0x0043 16*
  LCMOVE(); // LCMOVE
}


// ================================================
// 0x9369: WORD '>1ICONF' codep=0x224c wordp=0x9375 params=0 returns=0
// ================================================

void _gt_1ICONF() // >1ICONF
{
  Push(1);
  Push(pp_ICONFON); // ICONFON
  Store_2(); // !_2
  Push(pp_W5C1E); // W5C1E
  _099(); // 099
  Push(0);
  W934D(); // W934D
}


// ================================================
// 0x9385: WORD '>2ICONF' codep=0x224c wordp=0x9391 params=0 returns=0
// ================================================

void _gt_2ICONF() // >2ICONF
{
  Push(2);
  Push(pp_ICONFON); // ICONFON
  Store_2(); // !_2
  Push(2);
  Push(pp_W5C1E); // W5C1E
  Store_2(); // !_2
  Push(1);
  W934D(); // W934D
}


// ================================================
// 0x93a3: WORD '>3ICONF' codep=0x224c wordp=0x93af params=0 returns=0
// ================================================

void _gt_3ICONF() // >3ICONF
{
  Push(3);
  Push(pp_ICONFON); // ICONFON
  Store_2(); // !_2
  Push(3);
  Push(pp_W5C1E); // W5C1E
  Store_2(); // !_2
  Push(2);
  W934D(); // W934D
}


// ================================================
// 0x93c1: WORD 'W93C3' codep=0x93c3 wordp=0x93c3
// ================================================
// 0x93c3: pop    ax
// 0x93c4: push   es
// 0x93c5: mov    es,ax
// 0x93c7: mov    bx,[5BD2] // IINDEX
// 0x93cb: shl    bx,1
// 0x93cd: es:    
// 0x93ce: mov    ax,[bx]
// 0x93d0: pop    es
// 0x93d1: push   ax
// 0x93d2: lodsw
// 0x93d3: mov    bx,ax
// 0x93d5: jmp    word ptr [bx]

// ================================================
// 0x93d7: WORD 'W93D9' codep=0x93d9 wordp=0x93d9
// ================================================
// 0x93d9: pop    ax
// 0x93da: push   es
// 0x93db: mov    es,ax
// 0x93dd: mov    bx,[5BD2] // IINDEX
// 0x93e1: xor    ax,ax
// 0x93e3: es:    
// 0x93e4: mov    al,[bx]
// 0x93e6: pop    es
// 0x93e7: push   ax
// 0x93e8: lodsw
// 0x93e9: mov    bx,ax
// 0x93eb: jmp    word ptr [bx]

// ================================================
// 0x93ed: WORD '!IW' codep=0x93f5 wordp=0x93f5
// ================================================
// 0x93f5: pop    ax
// 0x93f6: pop    cx
// 0x93f7: push   es
// 0x93f8: mov    es,ax
// 0x93fa: mov    bx,[5BD2] // IINDEX
// 0x93fe: shl    bx,1
// 0x9400: es:    
// 0x9401: mov    [bx],cx
// 0x9403: pop    es
// 0x9404: lodsw
// 0x9405: mov    bx,ax
// 0x9407: jmp    word ptr [bx]

// ================================================
// 0x9409: WORD '!IB' codep=0x9411 wordp=0x9411
// ================================================
// 0x9411: pop    ax
// 0x9412: pop    cx
// 0x9413: push   es
// 0x9414: mov    es,ax
// 0x9416: mov    bx,[5BD2] // IINDEX
// 0x941a: es:    
// 0x941b: mov    [bx],cl
// 0x941d: pop    es
// 0x941e: lodsw
// 0x941f: mov    bx,ax
// 0x9421: jmp    word ptr [bx]

// ================================================
// 0x9423: WORD '@IX' codep=0x224c wordp=0x942b
// ================================================

void GetIX() // @IX
{
  Push(Read16(pp_W5B8E)); // W5B8E @
  W93C3(); // W93C3
}


// ================================================
// 0x9433: WORD '@IY' codep=0x224c wordp=0x943b
// ================================================

void GetIY() // @IY
{
  Push(Read16(pp_W5B92)); // W5B92 @
  W93C3(); // W93C3
}


// ================================================
// 0x9443: WORD '@ID' codep=0x224c wordp=0x944b
// ================================================

void GetID() // @ID
{
  Push(Read16(pp_W5B96)); // W5B96 @
  W93D9(); // W93D9
}


// ================================================
// 0x9453: WORD '@IC' codep=0x224c wordp=0x945b
// ================================================

void GetIC() // @IC
{
  Push(Read16(pp_W5B9A)); // W5B9A @
  W93D9(); // W93D9
}


// ================================================
// 0x9463: WORD '@IL' codep=0x224c wordp=0x946b
// ================================================

void GetIL() // @IL
{
  Push(Read16(pp_W5B9E)); // W5B9E @
  W93C3(); // W93C3
}


// ================================================
// 0x9473: WORD '@IH' codep=0x224c wordp=0x947b
// ================================================

void GetIH() // @IH
{
  Push(Read16(pp_IHSEG)); // IHSEG @
  W93D9(); // W93D9
}


// ================================================
// 0x9483: WORD '!IX' codep=0x224c wordp=0x948b
// ================================================

void StoreIX() // !IX
{
  Push(Read16(pp_W5B8E)); // W5B8E @
  StoreIW(); // !IW
}


// ================================================
// 0x9493: WORD '!IY' codep=0x224c wordp=0x949b
// ================================================

void StoreIY() // !IY
{
  Push(Read16(pp_W5B92)); // W5B92 @
  StoreIW(); // !IW
}


// ================================================
// 0x94a3: WORD '!ID' codep=0x224c wordp=0x94ab
// ================================================

void StoreID() // !ID
{
  Push(Read16(pp_W5B96)); // W5B96 @
  StoreIB(); // !IB
}


// ================================================
// 0x94b3: WORD '!IC' codep=0x224c wordp=0x94bb
// ================================================

void StoreIC() // !IC
{
  Push(Read16(pp_W5B9A)); // W5B9A @
  StoreIB(); // !IB
}


// ================================================
// 0x94c3: WORD '!IL' codep=0x224c wordp=0x94cb
// ================================================

void StoreIL() // !IL
{
  Push(Read16(pp_W5B9E)); // W5B9E @
  StoreIW(); // !IW
}


// ================================================
// 0x94d3: WORD '!IH' codep=0x224c wordp=0x94db
// ================================================

void StoreIH() // !IH
{
  Push(Read16(pp_IHSEG)); // IHSEG @
  StoreIB(); // !IB
}


// ================================================
// 0x94e3: WORD 'INIT-IC' codep=0x224c wordp=0x94ef params=0 returns=0
// ================================================

void INIT_dash_IC() // INIT-IC
{
  Push(pp_IGLOBAL); // IGLOBAL
  _099(); // 099
  Push(pp_ILOCAL); // ILOCAL
  _099(); // 099
}


// ================================================
// 0x94f9: WORD '.BACKGR' codep=0x224c wordp=0x9505
// ================================================

void DrawBACKGR() // .BACKGR
{
  Push(pp__i__dot_BACKG); // '.BACKG
  GetEXECUTE(); // @EXECUTE
}


// ================================================
// 0x950b: WORD '?VCLIP' codep=0x224c wordp=0x9516 params=2 returns=3
// ================================================

void IsVCLIP() // ?VCLIP
{
  unsigned short int a;
  Push(Read16(regsp)); // DUP
  Push(Read16(regsp)); // DUP
  Push(0x0098);
  _st_(); // <
  SWAP(); // SWAP
  Push(0xffd8);
  _gt_(); // >
  Push(Pop() & Pop()); // AND
  a = Pop(); // >R
  OVER(); // OVER
  Push(Read16(regsp)); // DUP
  Push(0x0066);
  _st_(); // <
  SWAP(); // SWAP
  Push(0xffd8);
  _gt_(); // >
  Push(Pop() & Pop()); // AND
  Push(Pop() & a); //  R> AND
}


// ================================================
// 0x9546: WORD 'W9548' codep=0x224c wordp=0x9548
// ================================================

void W9548() // W9548
{
  unsigned short int i, imax;
  Push(Read16(regsp)); // DUP
  Push(0x005a);
  _gt_(); // >
  if (Pop() != 0)
  {
    Push(Pop() - 0x0028); //  0x0028 -
  }
  Push(0);
  SWAP(); // SWAP
  ReadArray(ICONIMA); // ICONIMA
  Push(pp_ABLT); // ABLT
  Store_2(); // !_2
  Push(pp_BLTSEG); // BLTSEG
  Store_2(); // !_2

  i = 0;
  imax = 2;
  do // (DO)
  {
    Push(Read16(pp_BLTSEG)); // BLTSEG @
    Push(Read16(pp_ABLT)); // ABLT @
    LC_at_(); // LC@
    I_gt_C(); // I>C
    IsCGA(); // ?CGA
    if (Pop() != 0)
    {
      _gt_FLAG(); // >FLAG
    }
    StoreCOLOR(); // !COLOR
    Push(1);
    Push(pp_ABLT); // ABLT
    _plus__ex__2(); // +!_2
    BLT(); // BLT
    Push(8);
    Push(pp_ABLT); // ABLT
    _plus__ex__2(); // +!_2
    i++;
  } while(i<imax); // (LOOP)

}


// ================================================
// 0x9598: WORD 'W959A' codep=0x224c wordp=0x959a
// ================================================

void W959A() // W959A
{
  Push(8);
  Push(8);
  Push(pp_WBLT); // WBLT
  Store_2(); // !_2
  Push(pp_LBLT); // LBLT
  Store_2(); // !_2
  GetIX(); // @IX
  GetIY(); // @IY
  WLD_gt_SCR(); // WLD>SCR
  SCR_gt_BLT(); // SCR>BLT
  Push(pp_YBLT); // YBLT
  Store_2(); // !_2
  Push(pp_XBLT); // XBLT
  Store_2(); // !_2
  GetID(); // @ID
  W9548(); // W9548
  GetIC(); // @IC
  Push(Pop()==Read16(cc_DEAD_dash_IC)?1:0); //  DEAD-IC =
  if (Pop() != 0)
  {
    Push(0x0013);
    W9548(); // W9548
    return;
  }
  GetIC(); // @IC
  Push(Pop()==4?1:0); //  4 =
  if (Pop() == 0) return;
  Push(0x0012);
  W9548(); // W9548
}


// ================================================
// 0x95e0: WORD 'W95E2' codep=0x224c wordp=0x95e2
// ================================================

void W95E2() // W95E2
{
  Push(pp__i__dot_FLUX_dash_); // '.FLUX-
  GetEXECUTE(); // @EXECUTE
}


// ================================================
// 0x95e8: WORD '.CIRCLE_2' codep=0x224c wordp=0x95f4
// ================================================

void DrawCIRCLE_2() // .CIRCLE_2
{
  GetIC(); // @IC
  StoreCOLOR(); // !COLOR
  GetIX(); // @IX
  GetIY(); // @IY
  WLD_gt_SCR(); // WLD>SCR
  GetID(); // @ID
  Push((Pop() - Read16(cc_NULL_dash_IC)) * Read16(cc__star_MAPSCA)); //  NULL-IC - *MAPSCA *
  Push(pp_YWLD_c_YP); // YWLD:YP
  _2_at_(); // 2@
  Push(Read16(pp_CONTEXT_3)==3?1:0); // CONTEXT_3 @ 3 =
  if (Pop() != 0)
  {
    Push(Pop() + 7); //  7 +
  }
  SWAP(); // SWAP
  _star__slash_(); // */
  FILLCIR(); // FILLCIR
}


// ================================================
// 0x9624: WORD 'SYSCASE' codep=0x4b3b wordp=0x9630
// ================================================

void SYSCASE() // SYSCASE
{
  switch(Pop()) // SYSCASE
  {
  case 4:
    Push(pp_SSYSEG); // SSYSEG
    break;
  case 6:
    Push(pp_SSYSEG); // SSYSEG
    break;
  case 15:
    Push(pp_MSYSEG); // MSYSEG
    break;
  case 14:
    Push(pp_MSYSEG); // MSYSEG
    break;
  default:
    Push(pp_LSYSEG); // LSYSEG
    break;

  }
}

// ================================================
// 0x9644: WORD 'W9646' codep=0x224c wordp=0x9646
// ================================================

void W9646() // W9646
{
  unsigned short int i, imax;
  _1PIX(); // 1PIX
  GetIX(); // @IX
  GetIY(); // @IY
  WLD_gt_SCR(); // WLD>SCR

  i = 0;
  imax = 5;
  do // (DO)
  {
    RNDCLR(); // RNDCLR
    _2DUP(); // 2DUP
    Push(i); // I
    DrawCIRCLE_1(); // .CIRCLE_1
    i++;
  } while(i<imax); // (LOOP)

  Pop(); Pop(); // 2DROP
  _2PIX(); // 2PIX
}


// ================================================
// 0x9666: WORD 'W9668' codep=0x224c wordp=0x9668
// ================================================

void W9668() // W9668
{
  GetIC(); // @IC
  SYSCASE(); // SYSCASE case
  Push(Read16(Pop())); //  @
  Push(0);
  GetDS(); // @DS
  Push(pp_BMAP); // BMAP
  Push(0x000e);
  LCMOVE(); // LCMOVE
  GetIX(); // @IX
  GetIY(); // @IY
  WLD_gt_SCR(); // WLD>SCR
  BMHIGH(); // BMHIGH
  Push((Read16(Pop())&0xFF) >> 1); //  C@ 2/
  Push(Pop() + Pop()); // +
  SWAP(); // SWAP
  BMWIDE(); // BMWIDE
  Push(Read16(Pop())); //  @
  Push(4);
  _slash_(); // /
  _dash_(); // -
  SWAP(); // SWAP
  IsVCLIP(); // ?VCLIP
  if (Pop() != 0)
  {
    IsVGA(); // ?VGA
    if (Pop() != 0)
    {
      Push(0x001c);
      BMPAL(); // BMPAL
      C_ex__2(); // C!_2
    }
    DrawRAW(); // .RAW
    Push(0);
    BMPAL(); // BMPAL
    C_ex__2(); // C!_2
    return;
  }
  Pop(); Pop(); // 2DROP
}


// ================================================
// 0x96ba: WORD '.ICONCA' codep=0x4b3b wordp=0x96c6
// ================================================

void DrawICONCA() // .ICONCA
{
  switch(Pop()) // .ICONCA
  {
  case 253:
    W9668(); // W9668
    break;
  case 50:
    W9646(); // W9646
    break;
  case 255:
    W95E2(); // W95E2
    break;
  case 254:
    NOP(); // NOP
    break;
  default:
    DrawCIRCLE_2(); // .CIRCLE_2
    break;

  }
}

// ================================================
// 0x96da: WORD 'POINT>I' codep=0x224c wordp=0x96e6 params=1 returns=0
// ================================================

void POINT_gt_I() // POINT>I
{
  Push(pp_IINDEX); // IINDEX
  Store_2(); // !_2
}


// ================================================
// 0x96ec: WORD '.ICON' codep=0x224c wordp=0x96f6
// ================================================

void DrawICON() // .ICON
{
  GetIX(); // @IX
  GetIY(); // @IY
  IsINVIS(); // ?INVIS
  GetID(); // @ID
  Push(0x0033);
  Push(0x005b);
  WITHIN(); // WITHIN
  Push(Pop() | Pop()); // OR
  if (Pop() == 0) return;
  GetID(); // @ID
  Push(Read16(cc_NULL_dash_IC)); // NULL-IC
  _st_(); // <
  GetID(); // @ID
  Push(0x005b);
  Push(0x0063);
  WITHIN(); // WITHIN
  Push(Pop() | Pop()); // OR
  if (Pop() != 0)
  {
    W959A(); // W959A
    return;
  }
  GetID(); // @ID
  DrawICONCA(); // .ICONCA case
}


// ================================================
// 0x9732: WORD '.LOCAL-' codep=0x224c wordp=0x973e params=0 returns=0
// ================================================

void DrawLOCAL_dash_() // .LOCAL-
{
  unsigned short int i, imax;
  Push(Read16(pp_ILOCAL)); // ILOCAL @
  if (Read16(regsp) != 0) Push(Read16(regsp)); // ?DUP
  if (Pop() == 0) return;

  i = 0;
  imax = Pop();
  do // (DO)
  {
    Push(i); // I
    POINT_gt_I(); // POINT>I
    DrawICON(); // .ICON
    i++;
  } while(i<imax); // (LOOP)

}


// ================================================
// 0x9758: WORD 'W975A' codep=0x975a wordp=0x975a
// ================================================
// 0x975a: pop    cx
// 0x975b: pop    word ptr [5C90] // W5C90
// 0x975f: pop    word ptr [5C8C] // W5C8C
// 0x9763: pop    word ptr [5C88] // W5C88
// 0x9767: pop    word ptr [5C84] // W5C84
// 0x976b: xor    ax,ax
// 0x976d: push   ax
// 0x976e: or     cx,cx
// 0x9770: jle    97B3
// 0x9772: mov    bx,cx
// 0x9774: dec    bx
// 0x9775: add    bx,[5C90] // W5C90
// 0x9779: shl    bx,1
// 0x977b: push   es
// 0x977c: push   word ptr [5B8E] // W5B8E
// 0x9780: pop    es
// 0x9781: es:    
// 0x9782: mov    dx,[bx]
// 0x9784: pop    es
// 0x9785: sub    dx,[5C84] // W5C84
// 0x9789: jns    978D
// 0x978b: neg    dx
// 0x978d: cmp    dx,[5C8C] // W5C8C
// 0x9791: jg     97B1
// 0x9793: push   es
// 0x9794: push   word ptr [5B92] // W5B92
// 0x9798: pop    es
// 0x9799: es:    
// 0x979a: mov    dx,[bx]
// 0x979c: pop    es
// 0x979d: sub    dx,[5C88] // W5C88
// 0x97a1: jns    97A5
// 0x97a3: neg    dx
// 0x97a5: cmp    dx,[5C8C] // W5C8C
// 0x97a9: jg     97B1
// 0x97ab: pop    ax
// 0x97ac: shr    bx,1
// 0x97ae: push   bx
// 0x97af: inc    ax
// 0x97b0: push   ax
// 0x97b1: loop   9772
// 0x97b3: lodsw
// 0x97b4: mov    bx,ax
// 0x97b6: jmp    word ptr [bx]

// ================================================
// 0x97b8: WORD '?ICONS-_1' codep=0x224c wordp=0x97c4
// ================================================

void IsICONS_dash__1() // ?ICONS-_1
{
  Push(0);
  SWAP(); // SWAP
  W975A(); // W975A
}


// ================================================
// 0x97cc: WORD '?ICONS-_2' codep=0x224c wordp=0x97d8
// ================================================

void IsICONS_dash__2() // ?ICONS-_2
{
  Push(0);
  SWAP(); // SWAP
  IsICONS_dash__1(); // ?ICONS-_1
}


// ================================================
// 0x97e0: WORD '?ICON=I' codep=0x224c wordp=0x97ec params=1 returns=2
// ================================================

void IsICON_eq_I() // ?ICON=I
{
  unsigned short int a;
  Push(0);
  Push(Read16(pp_IGLOBAL)); // IGLOBAL @
  do
  {
    Push(Read16(pp_W5B9E)); // W5B9E @
    ROT(); // ROT
    ROT(); // ROT
    Push(5);
    PICK(); // PICK
    LWSCAN(); // LWSCAN
    if (Pop() != 0)
    {
      Push(Read16(pp_IHSEG)); // IHSEG @
      OVER(); // OVER
      Push(Pop() >> 1); //  2/
      LC_at_(); // LC@
      Push(3);
      PICK(); // PICK
      Push((Pop()==Pop())?1:0); // =
      if (Pop() != 0)
      {
        Push(1);
      } else
      {
        Push(Pop() + 2); //  2+
        Push(Read16(pp_IGLOBAL)); // IGLOBAL @
        OVER(); // OVER
        Push(Pop() >> 1); //  2/
        _dash_(); // -
        Push(0);
      }
    } else
    {
      Push(-1);
      Push(1);
    }
  } while(Pop() == 0);
  a = Pop(); // >R
  Pop(); Pop(); // 2DROP
  Push(a); // R>
  Push(Read16(regsp)); // DUP
  _0_st_(); // 0<
  if (Pop() != 0)
  {
    Pop(); // DROP
    Push(0);
    return;
  }
  Push(Pop() >> 1); //  2/
  Push(1);
}


// ================================================
// 0x9854: WORD 'W9856' codep=0x9856 wordp=0x9856
// ================================================
// 0x9856: pop    ax
// 0x9857: pop    bx
// 0x9858: push   es
// 0x9859: push   word ptr [5B96] // W5B96
// 0x985d: pop    es
// 0x985e: call   4AB8
// 0x9861: push   word ptr [5B9A] // W5B9A
// 0x9865: pop    es
// 0x9866: call   4AB8
// 0x9869: push   word ptr [5BAA] // IHSEG
// 0x986d: pop    es
// 0x986e: call   4AB8
// 0x9871: shl    ax,1
// 0x9873: shl    bx,1
// 0x9875: push   word ptr [5B8E] // W5B8E
// 0x9879: pop    es
// 0x987a: call   2F36
// 0x987d: push   word ptr [5B92] // W5B92
// 0x9881: pop    es
// 0x9882: call   2F36
// 0x9885: push   word ptr [5B9E] // W5B9E
// 0x9889: pop    es
// 0x988a: call   2F36
// 0x988d: pop    es
// 0x988e: lodsw
// 0x988f: mov    bx,ax
// 0x9891: jmp    word ptr [bx]

// ================================================
// 0x9893: WORD 'W9895' codep=0x224c wordp=0x9895 params=2 returns=0
// ================================================

void W9895() // W9895
{
  unsigned short int i, imax;
  Push(pp_W5C90); // W5C90
  Store_2(); // !_2
  if (Read16(regsp) != 0) Push(Read16(regsp)); // ?DUP
  if (Pop() == 0) return;

  i = 0;
  imax = Pop();
  do // (DO)
  {
    Push(i + Read16(pp_W5C90)); // I W5C90 @ +
    W9856(); // W9856
    i++;
  } while(i<imax); // (LOOP)

}


// ================================================
// 0x98b3: WORD '-ICON' codep=0x224c wordp=0x98bd params=0 returns=2
// ================================================

void _dash_ICON() // -ICON
{
  unsigned short int a;
  CI(); // CI
  IsICON_eq_I(); // ?ICON=I
  if (Pop() == 0) return;
  Push(Read16(pp_ILOCAL)); // ILOCAL @
  OVER(); // OVER
  _gt_(); // >
  if (Pop() != 0)
  {
    Push(-1);
    Push(pp_ILOCAL); // ILOCAL
    _plus__ex__2(); // +!_2
    Push(Read16(pp_ILOCAL)); // ILOCAL @
    Push(Read16(regsp)); // DUP
    a = Pop(); // >R
    W9856(); // W9856
    Push(a); // R>
  }
  Push(-1);
  Push(pp_IGLOBAL); // IGLOBAL
  _plus__ex__2(); // +!_2
  Push(Read16(pp_IGLOBAL)); // IGLOBAL @
  W9856(); // W9856
}


// ================================================
// 0x98f1: WORD 'W98F3' codep=0x98f3 wordp=0x98f3
// ================================================
// 0x98f3: pop    cx
// 0x98f4: pop    word ptr [5C90] // W5C90
// 0x98f8: pop    dx
// 0x98f9: pop    word ptr [49B6] // ZZZ
// 0x98fd: xor    ax,ax
// 0x98ff: push   es
// 0x9900: pop    word ptr [49B4] // ZZZ
// 0x9904: push   word ptr [5B96] // W5B96
// 0x9908: pop    es
// 0x9909: push   ax
// 0x990a: or     cx,cx
// 0x990c: jle    992A
// 0x990e: mov    bx,cx
// 0x9910: dec    bx
// 0x9911: add    bx,[5C90] // W5C90
// 0x9915: es:    
// 0x9916: mov    al,[bx]
// 0x9918: cmp    ax,dx
// 0x991a: jns    9928
// 0x991c: cmp    ax,[49B6] // ZZZ
// 0x9920: jle    9928
// 0x9922: pop    ax
// 0x9923: push   bx
// 0x9924: inc    ax
// 0x9925: push   ax
// 0x9926: xor    ax,ax
// 0x9928: loop   990E
// 0x992a: push   word ptr [49B4] // ZZZ
// 0x992e: pop    es
// 0x992f: lodsw
// 0x9930: mov    bx,ax
// 0x9932: jmp    word ptr [bx]

// ================================================
// 0x9934: WORD '?ICONSI' codep=0x224c wordp=0x9940
// ================================================

void IsICONSI() // ?ICONSI
{
  Push(0);
  SWAP(); // SWAP
  W98F3(); // W98F3
}


// ================================================
// 0x9948: WORD '+ICON_2' codep=0x224c wordp=0x9952
// ================================================

void _plus_ICON_2() // +ICON_2
{
  Push(Read16(pp_IGLOBAL)); // IGLOBAL @
  Push(Read16(regsp)); // DUP
  Push(Pop()==Read16(cc_W5286)?1:0); //  W5286 =
  if (Pop() != 0)
  {
    UNRAVEL(); // UNRAVEL
  }
  POINT_gt_I(); // POINT>I
  Push(1);
  Push(pp_IGLOBAL); // IGLOBAL
  _plus__ex__2(); // +!_2
  StoreIH(); // !IH
  StoreIL(); // !IL
  StoreIC(); // !IC
  StoreID(); // !ID
  StoreIY(); // !IY
  StoreIX(); // !IX
}


// ================================================
// 0x9978: WORD '+ICONBO' codep=0x224c wordp=0x9984 params=0 returns=0
// ================================================

void _plus_ICONBO() // +ICONBO
{
  Push(Read16(pp__i_ICONBO)); // 'ICONBO @
  MODULE(); // MODULE
}


// ================================================
// 0x998c: WORD 'ORGLIST' codep=0x224c wordp=0x9998 params=0 returns=0
// ================================================

void ORGLIST() // ORGLIST
{
  unsigned short int i, imax;
  Push(pp_ANCHOR); // ANCHOR
  _2_at_(); // 2@
  Push(Pop() - Read16(pp_YABS)); //  YABS @ -
  ABS(); // ABS
  SWAP(); // SWAP
  Push(Pop() - Read16(pp_XABS)); //  XABS @ -
  ABS(); // ABS
  MAX(); // MAX
  Push(Pop() - Read16(pp_LOCRADI)); //  LOCRADI @ -
  _0_gt_(); // 0>
  if (Pop() == 0) return;
  Push(Read16(pp_XABS)); // XABS @
  Push(Read16(pp_YABS)); // YABS @
  _2DUP(); // 2DUP
  Push(pp_ANCHOR); // ANCHOR
  _2_ex__2(); // 2!_2
  Push(Read16(pp_CONTEXT_3)); // CONTEXT_3 @
  if (Pop() != 0)
  {
    Push(Read16(pp_LOCRADI) + Read16(pp_W5658)); // LOCRADI @ W5658 @ +
    Push(Read16(pp_IGLOBAL)); // IGLOBAL @
    IsICONS_dash__1(); // ?ICONS-_1
    Push(Read16(regsp)); // DUP
    Push(pp_ILOCAL); // ILOCAL
    Store_2(); // !_2
    Push(0);
    W9895(); // W9895
    Push(Read16(cc_NULL_dash_IC)); // NULL-IC
    Push(Read16(cc_SYS_dash_ICO)); // SYS-ICO
    Push(Read16(pp_ILOCAL)); // ILOCAL @
    IsICONSI(); // ?ICONSI
    Push(Read16(regsp)); // DUP
    Push(pp_W55EE); // W55EE
    Store_2(); // !_2
    Push(0);
    W9895(); // W9895
    Push(pp_ANCHOR); // ANCHOR
    _2_at_(); // 2@
    Push(Read16(pp_LOCRADI)); // LOCRADI @
    Push(Read16(pp_W55EE)); // W55EE @
    Push(Read16(pp_ILOCAL)); // ILOCAL @
    OVER(); // OVER
    _dash_(); // -
    W975A(); // W975A
    Push(Read16(regsp)); // DUP
    Push(Pop() + Read16(pp_W55EE)); //  W55EE @ +
    Push(pp_ILOCAL); // ILOCAL
    Store_2(); // !_2
    Push(Read16(pp_W55EE)); // W55EE @
    W9895(); // W9895
  } else
  {
    Push(Read16(pp_LOCRADI)); // LOCRADI @
    Push(Read16(pp_IGLOBAL)); // IGLOBAL @
    IsICONS_dash__1(); // ?ICONS-_1
    Push(Read16(regsp)); // DUP
    Push(pp_ILOCAL); // ILOCAL
    Store_2(); // !_2
    Push(0);
    W9895(); // W9895
  }
  Push(0x0019);
  Push(0x0023);
  Push(Read16(pp_ILOCAL)); // ILOCAL @
  IsICONSI(); // ?ICONSI
  if (Read16(regsp) != 0) Push(Read16(regsp)); // ?DUP
  if (Pop() == 0) return;

  i = 0;
  imax = Pop();
  do // (DO)
  {
    Push(Read16(pp_ILOCAL) - 1); // ILOCAL @ 1-
    W9856(); // W9856
    i++;
  } while(i<imax); // (LOOP)

}


// ================================================
// 0x9a66: WORD 'SFILL' codep=0x224c wordp=0x9a70 params=0 returns=0
// ================================================

void SFILL() // SFILL
{
  BFILL(); // BFILL
}


// ================================================
// 0x9a74: WORD 'POS.' codep=0x224c wordp=0x9a7d params=2 returns=0
// ================================================

void POS_dot_() // POS.
{
  Push(pp_YBLT); // YBLT
  Store_2(); // !_2
  Push(pp_XBLT); // XBLT
  Store_2(); // !_2
}


// ================================================
// 0x9a87: WORD 'WINDOW' codep=0x224c wordp=0x9a92 params=4 returns=0
// ================================================

void WINDOW() // WINDOW
{
  Push(Read16(regsp)); // DUP
  Push(pp_WCHARS); // WCHARS
  Store_2(); // !_2
  Push(Pop() * 4); //  4 *
  ROT(); // ROT
  Push(Read16(regsp)); // DUP
  Push(pp_WLEFT); // WLEFT
  Store_2(); // !_2
  Push(Pop() + Pop()); // +
  Push(pp_WRIGHT); // WRIGHT
  Store_2(); // !_2
  Push(Read16(regsp)); // DUP
  Push(pp_WLINES); // WLINES
  Store_2(); // !_2
  Push((Pop() - 1) * 0xfff9 - 5); //  1- 0xfff9 * 5 -
  OVER(); // OVER
  Push(Pop() + Pop()); // +
  Push(pp_WBOTTOM); // WBOTTOM
  Store_2(); // !_2
  Push(pp_WTOP); // WTOP
  Store_2(); // !_2
}


// ================================================
// 0x9aca: WORD '.1LOGO' codep=0x224c wordp=0x9ad5
// ================================================

void Draw1LOGO() // .1LOGO
{
  unsigned short int a, b;
  GetCOLOR(); // @COLOR
  a = Pop(); // >R
  b = Pop(); // >R
  Push(Read16(b)); // R@
  StoreCOLOR(); // !COLOR
  POS_dot_(); // POS.
  GetDS(); // @DS
  Push(pp_BLTSEG); // BLTSEG
  Store_2(); // !_2
  Push(0x000a);
  Push(pp_LBLT); // LBLT
  Store_2(); // !_2
  Push(0x000a);
  Push(pp_WBLT); // WBLT
  Store_2(); // !_2
  Push(pp__1LOGO); // 1LOGO
  Push(pp_ABLT); // ABLT
  Store_2(); // !_2
  Push(b); // R>
  GetColor(GREY2);
  Push((Pop()==Pop())?1:0); // =
  if (Pop() != 0)
  {
    GetColor(GREY2);
    StoreCOLOR(); // !COLOR
  }
  BLT(); // BLT
  Push(a); // R>
  StoreCOLOR(); // !COLOR
}


// ================================================
// 0x9b13: WORD 'W9B15' codep=0x224c wordp=0x9b15 params=0 returns=0
// ================================================

void W9B15() // W9B15
{
  Push(pp_W688B); // W688B
  Push(pp_VIN); // VIN
  Store_2(); // !_2
  Push(pp_W68CD); // W68CD
  Push(pp_VOUT); // VOUT
  Store_2(); // !_2
  Push(pp_W690F); // W690F
  Push(pp_OIN); // OIN
  Store_2(); // !_2
  Push(pp_W6951); // W6951
  Push(pp_OOUT); // OOUT
  Store_2(); // !_2
}


// ================================================
// 0x9b2f: WORD 'W9B31' codep=0x224c wordp=0x9b31
// ================================================

void W9B31() // W9B31
{
  unsigned short int a;
  W9B15(); // W9B15
  StoreCOLOR(); // !COLOR
  Push(pp_W688B); // W688B
  a = Pop(); // >R
  Push(Read16(regsp)); // DUP
  Push(a + 2); // I 2+
  Store_2(); // !_2
  Push(a + 0x000e); // I 0x000e +
  Store_2(); // !_2
  Push(Read16(regsp)); // DUP
  Push(a); // I
  Store_2(); // !_2
  Push(a + 4); // I 4 +
  Store_2(); // !_2
  Push(Read16(regsp)); // DUP
  Push(a + 6); // I 6 +
  Store_2(); // !_2
  Push(a + 0x000a); // I 0x000a +
  Store_2(); // !_2
  Push(Read16(regsp)); // DUP
  Push(a + 8); // I 8 +
  Store_2(); // !_2
  Push(a + 0x000c); // R> 0x000c +
  Store_2(); // !_2
  Push(4);
  Push(pp__n_IN); // #IN
  Store_2(); // !_2
}


// ================================================
// 0x9b89: WORD 'POLY-WI' codep=0x224c wordp=0x9b95
// ================================================

void POLY_dash_WI() // POLY-WI
{
  W9B31(); // W9B31
  SCANPOL(); // SCANPOL
  LFILLPO(); // LFILLPO
}


// ================================================
// 0x9b9d: WORD '>SSCT' codep=0x224c wordp=0x9ba7 params=0 returns=0
// ================================================

void _gt_SSCT() // >SSCT
{
  Push(0x0032);
  Push(pp_CTTOP); // CTTOP
  Store_2(); // !_2
  Push(2);
  Push(pp_CTBOT); // CTBOT
  Store_2(); // !_2
  Push(0x0033);
  Push(3);
  Push(7);
  Push(0x0026);
  WINDOW(); // WINDOW
  Push(1);
  Push(pp_WBOTTOM); // WBOTTOM
  Store_2(); // !_2
  Push(0x009e);
  Push(pp_WRIGHT); // WRIGHT
  Store_2(); // !_2
}


// ================================================
// 0x9bd3: WORD '>TVCT' codep=0x224c wordp=0x9bdd params=0 returns=0
// ================================================

void _gt_TVCT() // >TVCT
{
  Push(0x0040);
  Push(3);
  Push(7);
  Push(0x0026);
  WINDOW(); // WINDOW
  Push(0x009e);
  Push(pp_WRIGHT); // WRIGHT
  Store_2(); // !_2
  Push(Read16(pp_WTOP) - 1); // WTOP @ 1-
  Push(pp_CTTOP); // CTTOP
  Store_2(); // !_2
  Push(0x000f);
  Push(Read16(regsp)); // DUP
  Push(pp_WBOTTOM); // WBOTTOM
  Store_2(); // !_2
  Push(pp_CTBOT); // CTBOT
  Store_2(); // !_2
}


// ================================================
// 0x9c0d: WORD 'CTPOS.' codep=0x224c wordp=0x9c18 params=2 returns=0
// ================================================

void CTPOS_dot_() // CTPOS.
{
  _2DUP(); // 2DUP
  Push(pp_CTY); // CTY
  Store_2(); // !_2
  Push(pp_CTX); // CTX
  Store_2(); // !_2
  SWAP(); // SWAP
  Push(Pop() * 4 + 4); //  4 * 4 +
  Push(Read16(pp_CTTOP)); // CTTOP @
  ROT(); // ROT
  Push(Pop() * 7); //  7 *
  _dash_(); // -
  POS_dot_(); // POS.
}


// ================================================
// 0x9c3c: WORD 'CTERASE' codep=0x224c wordp=0x9c48
// ================================================

void CTERASE() // CTERASE
{
  GetCOLOR(); // @COLOR
  Push(Read16(pp_WTOP)); // WTOP @
  Push(Read16(pp_WLEFT)); // WLEFT @
  Push(Read16(pp_WBOTTOM)); // WBOTTOM @
  Push(Read16(pp_WRIGHT) - 2); // WRIGHT @ 2 -
  GetColor(BLACK);
  POLY_dash_WI(); // POLY-WI
  Push(0);
  Push(0);
  CTPOS_dot_(); // CTPOS.
  StoreCOLOR(); // !COLOR
}


// ================================================
// 0x9c6c: WORD 'W9C6E' codep=0x224c wordp=0x9c6e params=0 returns=2
// ================================================

void W9C6E() // W9C6E
{
  Push(Read16(pp_BEEPTONE)); // BEEPTONE @
  Push(Read16(pp_BEEPMS)); // BEEPMS @
}


// ================================================
// 0x9c78: WORD 'W9C7A' codep=0x224c wordp=0x9c7a params=2 returns=0
// ================================================

void W9C7A() // W9C7A
{
  Push(pp_BEEPMS); // BEEPMS
  _st__ex__gt_(); // <!>
  Push(pp_BEEPTONE); // BEEPTONE
  _st__ex__gt_(); // <!>
}


// ================================================
// 0x9c84: WORD 'CLICK' codep=0x224c wordp=0x9c8e params=0 returns=0
// ================================================

void CLICK() // CLICK
{
  W9C6E(); // W9C6E
  Push(0x0064);
  Push(pp_BEEPMS); // BEEPMS
  Store_2(); // !_2
  Push(0x0032);
  Push(pp_BEEPTONE); // BEEPTONE
  Store_2(); // !_2
  BEEP(); // BEEP
  W9C7A(); // W9C7A
}


// ================================================
// 0x9ca6: WORD '>SND' codep=0x224c wordp=0x9caf params=3 returns=0
// ================================================

void _gt_SND() // >SND
{
  unsigned short int i, imax, j, jmax;
  Push(0);
  Push(Read16(pp_Get_co_0_star_1_sc_)); // @,0*1; @
  if (Pop() != 0)
  {
    Push(1);
    TONE(); // TONE
    BEEPON_1(); // BEEPON_1

    i = Pop();
    imax = Pop();
    do // (DO)
    {
      TONE(); // TONE
      MS(); // MS
      i++;
    } while(i<imax); // (LOOP)

    BEEPOFF(); // BEEPOFF
    return;
  }

  j = Pop();
  jmax = Pop();
  do // (DO)
  {
    Pop(); Pop(); // 2DROP
    j++;
  } while(j<jmax); // (LOOP)

}


// ================================================
// 0x9cd9: WORD 'BEEPON_2' codep=0x224c wordp=0x9ce4 params=0 returns=0
// ================================================

void BEEPON_2() // BEEPON_2
{
  Push(Read16(pp_Get_co_0_star_1_sc_)); // @,0*1; @
  if (Pop() == 0) return;
  BEEPON_1(); // BEEPON_1
}


// ================================================
// 0x9cf0: WORD 'W9CF2' codep=0x224c wordp=0x9cf2 params=1 returns=2
// ================================================

void W9CF2() // W9CF2
{
  Push(Read16(cc_MUSSEG)); // MUSSEG
  SWAP(); // SWAP
}


// ================================================
// 0x9cf8: WORD 'W9CFA' codep=0x224c wordp=0x9cfa params=2 returns=0
// ================================================

void W9CFA() // W9CFA
{
  W9CF2(); // W9CF2
  L_ex_(); // L!
}


// ================================================
// 0x9d00: WORD 'W9D02' codep=0x224c wordp=0x9d02 params=1 returns=1
// ================================================

void W9D02() // W9D02
{
  W9CF2(); // W9CF2
  L_at_(); // L@
}


// ================================================
// 0x9d08: WORD 'W9D0A' codep=0x224c wordp=0x9d0a params=1 returns=1
// ================================================

void W9D0A() // W9D0A
{
  W9CF2(); // W9CF2
  LC_at_(); // LC@
}


// ================================================
// 0x9d10: WORD 'W9D12' codep=0x224c wordp=0x9d12 params=2 returns=0
// ================================================

void W9D12() // W9D12
{
  W9CF2(); // W9CF2
  LC_ex_(); // LC!
}


// ================================================
// 0x9d18: WORD 'W9D1A' codep=0x224c wordp=0x9d1a params=0 returns=0
// ================================================

void W9D1A() // W9D1A
{
  Push(1);
  Push(2);
  W9D12(); // W9D12
}


// ================================================
// 0x9d22: WORD 'W9D24' codep=0x224c wordp=0x9d24 params=0 returns=0
// ================================================

void W9D24() // W9D24
{
  Push(0);
  Push(2);
  W9D12(); // W9D12
  BEEPOFF(); // BEEPOFF
}


// ================================================
// 0x9d2e: WORD 'SONG' codep=0x224c wordp=0x9d37 params=1 returns=0
// ================================================

void SONG() // SONG
{
  Push(Read16(pp_Get_co_0_star_1_sc_)); // @,0*1; @
  if (Pop() != 0)
  {
    W9D24(); // W9D24
    Push(Pop() * 2 + 0x0010); //  2* 0x0010 +
    W9D02(); // W9D02
    Push(Read16(regsp)); // DUP
    Push(Read16(regsp)); // DUP
    Push(3);
    W9CFA(); // W9CFA
    Push(5);
    W9CFA(); // W9CFA
    Push(Read16(regsp)); // DUP
    W9D0A(); // W9D0A
    Push(9);
    W9D12(); // W9D12
    Push(Pop() + 1); //  1+
    W9D02(); // W9D02
    Push(7);
    W9CFA(); // W9CFA
    Push(1);
    Push(0x000a);
    W9D12(); // W9D12
    Push(1);
    Push(0x000d);
    W9D12(); // W9D12
    W9D1A(); // W9D1A
    return;
  }
  Pop(); // DROP
}


// ================================================
// 0x9d81: WORD '@CRS' codep=0x224c wordp=0x9d8a params=0 returns=11
// ================================================

void GetCRS() // @CRS
{
  Push(Read16(pp_YBLT)); // YBLT @
  Push(Read16(pp_XBLT)); // XBLT @
  Push(Read16(pp_LBLT)); // LBLT @
  Push(Read16(pp_WBLT)); // WBLT @
  Push(Read16(pp_ABLT)); // ABLT @
  Push(Read16(pp_NCRS)); // NCRS @
  Push(Read16(pp_OCRS)); // OCRS @
  GetCOLOR(); // @COLOR
  Push(Read16(pp_DCOLOR)); // DCOLOR @
  Push(Read16(pp_XORMODE)); // XORMODE @
  Push(Read16(pp_BLTSEG)); // BLTSEG @
}


// ================================================
// 0x9db6: WORD '!CRS' codep=0x224c wordp=0x9dbf
// ================================================

void StoreCRS() // !CRS
{
  Push(pp_BLTSEG); // BLTSEG
  Store_2(); // !_2
  Push(pp_XORMODE); // XORMODE
  Store_2(); // !_2
  Pop(); // DROP
  StoreCOLOR(); // !COLOR
  Push(pp_OCRS); // OCRS
  Store_2(); // !_2
  Push(pp_NCRS); // NCRS
  Store_2(); // !_2
  Push(pp_ABLT); // ABLT
  Store_2(); // !_2
  Push(pp_WBLT); // WBLT
  Store_2(); // !_2
  Push(pp_LBLT); // LBLT
  Store_2(); // !_2
  Push(pp_XBLT); // XBLT
  Store_2(); // !_2
  Push(pp_YBLT); // YBLT
  Store_2(); // !_2
}


// ================================================
// 0x9de9: WORD '$.' codep=0x224c wordp=0x9df0 params=1 returns=0
// ================================================

void _do__dot_() // $.
{
  COUNT(); // COUNT
  Exec("TYPE"); // call of word 0x2690 '(TYPE)'
}


// ================================================
// 0x9df6: WORD 'POLY-ER' codep=0x224c wordp=0x9e02
// ================================================

void POLY_dash_ER() // POLY-ER
{
  unsigned short int a, b;
  a = Pop(); // >R
  b = Pop(); // >R
  GetCRS(); // @CRS
  Push(Read16(pp_YBLT)); // YBLT @
  Push(Read16(regsp)); // DUP
  Push(Pop() - 4); //  4 -
  Push(Read16(pp_XBLT)); // XBLT @
  SWAP(); // SWAP
  OVER(); // OVER
  Push(Pop() + b * 4); //  R> 4 * +
  Push(a); // R>
  POLY_dash_WI(); // POLY-WI
  StoreCRS(); // !CRS
}


// ================================================
// 0x9e2a: WORD 'POS.PXT' codep=0x224c wordp=0x9e36
// ================================================

void POS_dot_PXT() // POS.PXT
{
  unsigned short int a, b, c, d;
  a = Pop(); // >R
  b = Pop(); // >R
  c = Pop(); // >R
  d = Pop(); // >R
  GetCRS(); // @CRS
  Push(d); // R>
  Push(c); // R>
  Push(b); // R>
  Push(a); // R>
  SWAP(); // SWAP
  POS_dot_(); // POS.
  POLY_dash_ER(); // POLY-ER
  StoreCRS(); // !CRS
}


// ================================================
// 0x9e52: WORD 'W9E54' codep=0x224c wordp=0x9e54
// ================================================

void W9E54() // W9E54
{
  unsigned short int a;
  a = Pop(); // >R
  GetCRS(); // @CRS
  Push(pp_XORMODE); // XORMODE
  _099(); // 099
  _gt_1FONT(); // >1FONT
  Push(Read16(pp_WLEFT)); // WLEFT @
  Push(Read16(pp_WBOTTOM) + 5); // WBOTTOM @ 5 +
  POS_dot_(); // POS.
  Push(a); // R>
  _do__dot_(); // $.
  StoreCRS(); // !CRS
}


// ================================================
// 0x9e74: WORD 'W9E76' codep=0x224c wordp=0x9e76
// ================================================

void W9E76() // W9E76
{
  unsigned short int a;
  a = Pop(); // >R
  GetCRS(); // @CRS
  Push(pp_XORMODE); // XORMODE
  _099(); // 099
  _gt_1FONT(); // >1FONT
  Push(Read16(pp_WLEFT)); // WLEFT @
  Push(Read16(pp_WTOP)); // WTOP @
  POS_dot_(); // POS.
  Push(a); // R>
  _do__dot_(); // $.
  StoreCRS(); // !CRS
}


// ================================================
// 0x9e92: WORD 'WUP' codep=0x224c wordp=0x9e9a
// ================================================

void WUP() // WUP
{
  unsigned short int i, imax;

  i = 0;
  imax = 7;
  do // (DO)
  {
    Push(Read16(pp_WLEFT)); // WLEFT @
    Push(Read16(pp_WTOP) - 1); // WTOP @ 1-
    Push(Read16(pp_WRIGHT)); // WRIGHT @
    Push(Read16(pp_WBOTTOM)); // WBOTTOM @
    Push(Read16(pp_WLEFT)); // WLEFT @
    Push(Read16(pp_WTOP)); // WTOP @
    LCOPYBL(); // LCOPYBL
    i++;
  } while(i<imax); // (LOOP)

}


// ================================================
// 0x9ec2: WORD 'WDN' codep=0x224c wordp=0x9eca
// ================================================

void WDN() // WDN
{
  unsigned short int i, imax;

  i = 0;
  imax = 7;
  do // (DO)
  {
    Push(Read16(pp_WLEFT)); // WLEFT @
    Push(Read16(pp_WTOP) + 1); // WTOP @ 1+
    Push(Read16(pp_WRIGHT)); // WRIGHT @
    Push(Read16(pp_WBOTTOM)); // WBOTTOM @
    Push(Read16(pp_WLEFT)); // WLEFT @
    Push(Read16(pp_WTOP)); // WTOP @
    LCOPYBL(); // LCOPYBL
    i++;
  } while(i<imax); // (LOOP)

}


// ================================================
// 0x9ef2: WORD 'WLINE-U' codep=0x224c wordp=0x9efe
// ================================================

void WLINE_dash_U() // WLINE-U
{
  WUP(); // WUP
  W9E54(); // W9E54
}


// ================================================
// 0x9f04: WORD 'WLINE-D' codep=0x224c wordp=0x9f10
// ================================================

void WLINE_dash_D() // WLINE-D
{
  WDN(); // WDN
  W9E76(); // W9E76
}


// ================================================
// 0x9f16: WORD 'GCR' codep=0x224c wordp=0x9f1e params=0 returns=0
// ================================================

void GCR() // GCR
{
  Push(0xfff9);
  Push(pp_YBLT); // YBLT
  _plus__ex__2(); // +!_2
  Push(Read16(pp_WLEFT)); // WLEFT @
  Push(pp_XBLT); // XBLT
  Store_2(); // !_2
}


// ================================================
// 0x9f30: WORD 'WSHORTE' codep=0x224c wordp=0x9f3c
// ================================================

void WSHORTE() // WSHORTE
{
  Push(Read16(pp_WCHARS)); // WCHARS @
  SWAP(); // SWAP
  Push(7);
  Push(pp_WBOTTOM); // WBOTTOM
  _plus__ex__2(); // +!_2
  Push(Read16(pp_WBOTTOM) - 2); // WBOTTOM @ 2-
  Push(Read16(pp_WLEFT)); // WLEFT @
  POS_dot_PXT(); // POS.PXT
  Push(-1);
  Push(pp_WLINES); // WLINES
  _plus__ex__2(); // +!_2
}


// ================================================
// 0x9f5c: WORD 'W9F5E' codep=0x224c wordp=0x9f5e params=0 returns=2
// ================================================

void W9F5E() // W9F5E
{
  Push2Words("NULL");
  Push(pp_FTRIG); // FTRIG
  OFF_2(); // OFF_2
}


// ================================================
// 0x9f66: WORD 'W9F68' codep=0x224c wordp=0x9f68
// ================================================

void W9F68() // W9F68
{
  Push(pp__i_BOSS); // 'BOSS
  GetEXECUTE(); // @EXECUTE
  W9F5E(); // W9F5E
}


// ================================================
// 0x9f70: WORD '>BOTT' codep=0x224c wordp=0x9f7a params=0 returns=0
// ================================================

void _gt_BOTT() // >BOTT
{
  Push(4);
  Push(Read16(pp_WBOTTOM) + 5); // WBOTTOM @ 5 +
  Push(Read16(regsp)); // DUP
  Push(0x000c);
  _st_(); // <
  if (Pop() != 0)
  {
    Push(Pop() + 1); //  1+
  }
  POS_dot_(); // POS.
}


// ================================================
// 0x9f96: WORD 'CTCR' codep=0x224c wordp=0x9f9f params=0 returns=0
// ================================================

void CTCR() // CTCR
{
  Push(0);
  Push(Read16(pp_CTY) + 1); // CTY @ 1+
  Push(6);
  MIN(); // MIN
  CTPOS_dot_(); // CTPOS.
}


// ================================================
// 0x9faf: WORD 'TTY-SCR' codep=0x224c wordp=0x9fbb
// ================================================

void TTY_dash_SCR() // TTY-SCR
{
  WUP(); // WUP
}


// ================================================
// 0x9fbf: WORD 'W9FC1' codep=0x224c wordp=0x9fc1
// ================================================

void W9FC1() // W9FC1
{
  _gt_1FONT(); // >1FONT
  TTY_dash_SCR(); // TTY-SCR
  _gt_BOTT(); // >BOTT
}


// ================================================
// 0x9fc9: WORD 'CTINIT' codep=0x224c wordp=0x9fd4
// ================================================

void CTINIT() // CTINIT
{
  _gt_1FONT(); // >1FONT
  Push(pp_XORMODE); // XORMODE
  _099(); // 099
  GetColor(WHITE);
  StoreCOLOR(); // !COLOR
  Push(0x9fc1); // 'W9FC1'
  Push(0x0644); // 'CR'
  EXECUTES(); // EXECUTES
}


// ================================================
// 0x9fea: WORD '.TTY' codep=0x224c wordp=0x9ff3
// ================================================

void DrawTTY() // .TTY
{
  W9FC1(); // W9FC1
  Exec("TYPE"); // call of word 0x2690 '(TYPE)'
}


// ================================================
// 0x9ff9: WORD 'VIEWSCR' codep=0x224c wordp=0xa005
// ================================================

void VIEWSCR() // VIEWSCR
{
  unsigned short int i, imax, j, jmax;

  i = 0;
  imax = 3;
  do // (DO)
  {
    Push(i + 1); // I 1+
    Push(0x00c3);
    OVER(); // OVER
    Push(0x0044);
    LLINE(); // LLINE
    Push(0x004c + i); // 0x004c I +
    Push(0x00c3);
    OVER(); // OVER
    Push(0x0044);
    LLINE(); // LLINE
    i++;
  } while(i<imax); // (LOOP)


  j = 0;
  jmax = 4;
  do // (DO)
  {
    Push(4);
    Push(0x0044 + j); // 0x0044 I +
    Push(0x004b);
    OVER(); // OVER
    LLINE(); // LLINE
    Push(4);
    Push(0x00c0 + j); // 0x00c0 I +
    Push(0x000b);
    OVER(); // OVER
    LLINE(); // LLINE
    Push(0x004b);
    Push(0x00c0 + j); // 0x00c0 I +
    Push(0x0044);
    OVER(); // OVER
    LLINE(); // LLINE
    j++;
  } while(j<jmax); // (LOOP)

  Push(0x004a);
  Push(0x00c0);
  Push(0x000c);
  OVER(); // OVER
  LLINE(); // LLINE
  GetCOLOR(); // @COLOR
  SWAP(); // SWAP
  StoreCOLOR(); // !COLOR
  _gt_MAINVI(); // >MAINVI
  BFILL(); // BFILL
  _gt_DISPLA(); // >DISPLA
  V_gt_DISPL(); // V>DISPL
  StoreCOLOR(); // !COLOR
}


// ================================================
// 0xa097: WORD 'TXT-WIN' codep=0x224c wordp=0xa0a3 params=0 returns=0
// ================================================

void TXT_dash_WIN() // TXT-WIN
{
  unsigned short int i, imax;
  Push(Read16(pp_IsON_dash_PLA) * 0x000d); // ?ON-PLA @ 0x000d *
  _gt_V(); // >V

  i = 0;
  imax = 2;
  do // (DO)
  {
    Push(i + 1); // I 1+
    VI(); // VI
    OVER(); // OVER
    Push(0x0034);
    VI(); // VI
    Push(Pop() + Pop()); // +
    LLINE(); // LLINE
    Push(0x009d + i); // 0x009d I +
    VI(); // VI
    OVER(); // OVER
    Push(0x0034);
    VI(); // VI
    Push(Pop() + Pop()); // +
    LLINE(); // LLINE
    i++;
  } while(i<imax); // (LOOP)

  Push(3);
  Push(0x0034);
  VI(); // VI
  Push(Pop() + Pop()); // +
  Push(0x009c);
  OVER(); // OVER
  LLINE(); // LLINE
  Push(3);
  VI(); // VI
  Push(0x009c);
  V_gt_(); // V>
  LLINE(); // LLINE
}


// ================================================
// 0xa101: WORD 'AUXSCRE' codep=0x224c wordp=0xa10d params=0 returns=0
// ================================================

void AUXSCRE() // AUXSCRE
{
  unsigned short int i, imax;

  i = 0;
  imax = 2;
  do // (DO)
  {
    Push(0x0051 + i); // 0x0051 I +
    Push(0x007d);
    OVER(); // OVER
    Push(0x00c6);
    LLINE(); // LLINE
    Push(0x009d + i); // 0x009d I +
    Push(0x007d);
    OVER(); // OVER
    Push(0x00c6);
    LLINE(); // LLINE
    i++;
  } while(i<imax); // (LOOP)

  Push(0x0053);
  Push(0x00c6);
  Push(0x009c);
  OVER(); // OVER
  LLINE(); // LLINE
  Push(0x0053);
  Push(0x007d);
  Push(0x009c);
  OVER(); // OVER
  LLINE(); // LLINE
}


// ================================================
// 0xa161: WORD 'BTN-WIN' codep=0x224c wordp=0xa16d params=0 returns=0
// ================================================

void BTN_dash_WIN() // BTN-WIN
{
  unsigned short int i, imax;

  i = 0;
  imax = 2;
  do // (DO)
  {
    Push(0x005a + i); // 0x005a I +
    Push(0x0037);
    OVER(); // OVER
    Push(0x007a);
    LLINE(); // LLINE
    Push(0x009d + i); // 0x009d I +
    Push(0x0037);
    OVER(); // OVER
    Push(0x007a);
    LLINE(); // LLINE
    i++;
  } while(i<imax); // (LOOP)

  Push(0x005c);
  Push(0x007a);
  Push(0x009c);
  OVER(); // OVER
  LLINE(); // LLINE
  Push(0x005c);
  Push(0x0037);
  Push(0x009c);
  OVER(); // OVER
  LLINE(); // LLINE
}


// ================================================
// 0xa1c1: WORD '.BUTTON' codep=0x224c wordp=0xa1cd
// ================================================

void DrawBUTTON() // .BUTTON
{
  unsigned short int i, imax;
  Push(pp_W547D); // W547D
  Push(pp_ABLT); // ABLT
  Store_2(); // !_2
  Push(9);
  Push(pp_LBLT); // LBLT
  Store_2(); // !_2
  Push(8);
  Push(pp_WBLT); // WBLT
  Store_2(); // !_2
  Push(pp_XORMODE); // XORMODE
  _099(); // 099
  GetDS(); // @DS
  Push(pp_BLTSEG); // BLTSEG
  Store_2(); // !_2

  i = 0;
  imax = 6;
  do // (DO)
  {
    Push(0x0051);
    Push(0x0078 - i * 0x000b); // 0x0078 I 0x000b * -
    POS_dot_(); // POS.
    BLT(); // BLT
    i++;
  } while(i<imax); // (LOOP)

  Push(2);
  Push(0x0041);
  GetColor(GREY2);
  Draw1LOGO(); // .1LOGO
}


// ================================================
// 0xa215: WORD '?MRC' codep=0x224c wordp=0xa21e
// ================================================

void IsMRC() // ?MRC
{
  Push(Read16(pp_MONITOR)); // MONITOR @
  Push(Read16(regsp)); // DUP
  Push(Pop()==3?1:0); //  3 =
  SWAP(); // SWAP
  Push(4);
  _gt_(); // >
  Push(Pop() | Pop()); // OR
  IsCGA(); // ?CGA
  Push(!Pop()); //  NOT
  Push(Pop() & Pop()); // AND
  if (Pop() != 0)
  {
    ROT(); // ROT
    ROT(); // ROT
  } else
  {
    IsCGA(); // ?CGA
    if (Pop() != 0)
    {
      ROT(); // ROT
    }
  }
  Pop(); Pop(); // 2DROP
}


// ================================================
// 0xa24e: WORD 'WA250' codep=0x224c wordp=0xa250
// ================================================

void WA250() // WA250
{
  unsigned short int a;
  a = Pop(); // >R
  GetCOLOR(); // @COLOR
  Push(0x0079);
  Push(0x005c);
  Push(0x0038);
  Push(0x009c);
  Push(a); // R>
  POLY_dash_WI(); // POLY-WI
  StoreCOLOR(); // !COLOR
}


// ================================================
// 0xa26c: WORD 'ERASE-A' codep=0x224c wordp=0xa278
// ================================================

void ERASE_dash_A() // ERASE-A
{
  unsigned short int a;
  a = Pop(); // >R
  GetCOLOR(); // @COLOR
  Push(0x00c5);
  Push(0x0053);
  Push(0x007e);
  Push(0x009c);
  Push(a); // R>
  POLY_dash_WI(); // POLY-WI
  StoreCOLOR(); // !COLOR
}


// ================================================
// 0xa294: WORD 'ERASE-T' codep=0x224c wordp=0xa2a0
// ================================================

void ERASE_dash_T() // ERASE-T
{
  unsigned short int a;
  a = Pop(); // >R
  GetCOLOR(); // @COLOR
  Push(0x0033);
  Push(3);
  Push(1);
  Push(0x009c);
  Push(a); // R>
  POLY_dash_WI(); // POLY-WI
  Push(0);
  Push(0);
  CTPOS_dot_(); // CTPOS.
  StoreCOLOR(); // !COLOR
}


// ================================================
// 0xa2be: WORD 'WA2C0' codep=0x224c wordp=0xa2c0
// ================================================

void WA2C0() // WA2C0
{
  Push(Pop() * 0x000b); //  0x000b *
  Push(0x0052);
  Push(0x0077);
  ROT(); // ROT
  _dash_(); // -
  POS_dot_(); // POS.
  StoreCOLOR(); // !COLOR
  Push(pp_XORMODE); // XORMODE
  _099(); // 099
}


// ================================================
// 0xa2dc: WORD '.HIGHLI' codep=0x224c wordp=0xa2e8
// ================================================

void DrawHIGHLI() // .HIGHLI
{
  GetDS(); // @DS
  Push(pp_BLTSEG); // BLTSEG
  Store_2(); // !_2
  WA2C0(); // WA2C0
  Push(pp_W5489); // W5489
  Push(pp_ABLT); // ABLT
  Store_2(); // !_2
  Push(7);
  Push(pp_LBLT); // LBLT
  Store_2(); // !_2
  Push(6);
  Push(pp_WBLT); // WBLT
  Store_2(); // !_2
  BLT(); // BLT
}


// ================================================
// 0xa306: WORD 'WA308' codep=0x224c wordp=0xa308
// ================================================

void WA308() // WA308
{
  GetDS(); // @DS
  Push(pp_BLTSEG); // BLTSEG
  Store_2(); // !_2
  WA2C0(); // WA2C0
  Push(-2);
  Push(pp_YBLT); // YBLT
  _plus__ex__2(); // +!_2
  Push(2);
  Push(pp_XBLT); // XBLT
  _plus__ex__2(); // +!_2
  Push(3);
  Push(pp_LBLT); // LBLT
  Store_2(); // !_2
  Push(2);
  Push(pp_WBLT); // WBLT
  Store_2(); // !_2
  Push(pp_W5491); // W5491
  Push(pp_ABLT); // ABLT
  Store_2(); // !_2
  BLT(); // BLT
}


// ================================================
// 0xa332: WORD '.ON' codep=0x224c wordp=0xa33a
// ================================================

void DrawON() // .ON
{
  unsigned short int a;
  OVER(); // OVER
  a = Pop(); // >R
  WA308(); // WA308
  Push(a); // R>
  GetColor(BLACK);
  Push((Pop()==Pop())?1:0); // =
  Push(!Pop()); //  NOT
  if (Pop() == 0) return;
  CLICK(); // CLICK
}


// ================================================
// 0xa350: WORD 'CLR-BUT' codep=0x224c wordp=0xa35c
// ================================================

void CLR_dash_BUT() // CLR-BUT
{
  unsigned short int i, imax;

  i = 0;
  imax = 6;
  do // (DO)
  {
    Push(0);
    Push(i); // I
    _2DUP(); // 2DUP
    DrawHIGHLI(); // .HIGHLI
    WA308(); // WA308
    i++;
  } while(i<imax); // (LOOP)

}


// ================================================
// 0xa372: WORD 'INIT-BU' codep=0x224c wordp=0xa37e
// ================================================

void INIT_dash_BU() // INIT-BU
{
  CLR_dash_BUT(); // CLR-BUT
  Push(pp_THIS_dash_BU); // THIS-BU
  _099(); // 099
  GetColor(BLUE);
  Push(0);
  DrawHIGHLI(); // .HIGHLI
}


// ================================================
// 0xa38c: WORD '(SHIP-C' codep=0x224c wordp=0xa398
// ================================================

void _ro_SHIP_dash_C() // (SHIP-C
{
  _gt_SSCT(); // >SSCT
  GetColor(GREY1);
  StoreCOLOR(); // !COLOR
  SFILL(); // SFILL
  GetColor(GREEN);
  GetColor(GREEN);
  GetColor(DK_dash_BLUE);
  IsMRC(); // ?MRC
  StoreCOLOR(); // !COLOR
  VIEWSCR(); // VIEWSCR
  AUXSCRE(); // AUXSCRE
  BTN_dash_WIN(); // BTN-WIN
  TXT_dash_WIN(); // TXT-WIN
  DrawBUTTON(); // .BUTTON
  Push(0);
  WA250(); // WA250
  GetColor(BLACK);
  ERASE_dash_A(); // ERASE-A
  CTERASE(); // CTERASE
  INIT_dash_BU(); // INIT-BU
}


// ================================================
// 0xa3c2: WORD 'SHIP-CO' codep=0x224c wordp=0xa3ce
// ================================================

void SHIP_dash_CO() // SHIP-CO
{
  GetColor(BLACK);
  _ro_SHIP_dash_C(); // (SHIP-C
  CTINIT(); // CTINIT
}


// ================================================
// 0xa3d6: WORD ''KEY' codep=0x224c wordp=0xa3df
// ================================================

void _i_KEY() // 'KEY
{
  Exec("IsTERMINAL"); // call of word 0x25bc '(?TERMINAL)'
  if (Pop() != 0)
  {
    Push(Read16(pp_LKEY)); // LKEY @
    Push(pp_SKEY); // SKEY
    Store_2(); // !_2
    KEY_2(); // KEY_2
    Push(Read16(regsp)); // DUP
    Push(pp_LKEY); // LKEY
    Store_2(); // !_2
    Push(Read16(regsp)); // DUP
    Push((Pop()==0x001b?1:0) & Read16(pp_ESC_dash_EN)); //  0x001b = ESC-EN @ AND
    Push(Read16(pp_CONTEXT_3)==5?1:0); // CONTEXT_3 @ 5 =
    if (Pop() != 0)
    {
      Is1DRV(); // ?1DRV
      Push(!Pop()); //  NOT
      Push(Pop() & Pop()); // AND
    }
    if (Pop() != 0)
    {
      SAVE_dash_OV(); // SAVE-OV
      Pop(); // DROP
      Push(0);
      Push(pp_ESC_dash_PFA); // ESC-PFA
      GetEXECUTE(); // @EXECUTE
    }
    Push(Read16(regsp)); // DUP
    Push(Pop()==0x0013?1:0); //  0x0013 =
    if (Pop() != 0)
    {
      Pop(); // DROP
      Push(0);
      Push(pp_Get_co_0_star_1_sc_); // @,0*1;
      Push(1);
      TOGGLE(); // TOGGLE
      Push(Read16(pp_Get_co_0_star_1_sc_)==0?1:0); // @,0*1; @ 0=
      if (Pop() != 0)
      {
        W9D24(); // W9D24
      }
    }
    Push(pp_KEYTIME); // KEYTIME
    _2_at_(); // 2@
    Push(pp_LKEYTIM); // LKEYTIM
    _2_ex__2(); // 2!_2
    TIME(); // TIME
    _2_at_(); // 2@
    Push(pp_KEYTIME); // KEYTIME
    _2_ex__2(); // 2!_2
    return;
  }
  Push(0);
}


// ================================================
// 0xa45d: WORD 'WA45F' codep=0x224c wordp=0xa45f params=0 returns=2
// ================================================

void WA45F() // WA45F
{
  Push(1);
  Push(-1);
}


// ================================================
// 0xa465: WORD 'WA467' codep=0x224c wordp=0xa467 params=0 returns=2
// ================================================

void WA467() // WA467
{
  Push(1);
  Push(0);
}


// ================================================
// 0xa46d: WORD 'WA46F' codep=0x224c wordp=0xa46f params=0 returns=2
// ================================================

void WA46F() // WA46F
{
  Push(1);
  Push(1);
}


// ================================================
// 0xa475: WORD 'WA477' codep=0x224c wordp=0xa477 params=0 returns=2
// ================================================

void WA477() // WA477
{
  Push(0);
  Push(-1);
  Push(pp_FQUIT); // FQUIT
  ON_2(); // ON_2
}


// ================================================
// 0xa481: WORD 'NOF' codep=0x1d29 wordp=0xa489
// ================================================
// 0xa489: db 0x00 0x00 '  '

// ================================================
// 0xa48b: WORD 'FKEY' codep=0x224c wordp=0xa494
// ================================================

void FKEY() // FKEY
{
  Exec("CREATE"); // call of word 0x1cbb '(CREATE)'
  _co_(); // ,
  CODE(); // (;CODE) inlined assembler code
// 0xa49a: call   1649
  Push(Read16(pp_NOF)); // NOF @
  if (Pop() != 0)
  {
    Pop(); // DROP
    Push2Words("NULL");
    return;
  }
  GetEXECUTE(); // @EXECUTE
}


// ================================================
// 0xa4b1: WORD 'WA4B3' codep=0x224c wordp=0xa4b3 params=0 returns=2
// ================================================
// orphan

void WA4B3() // WA4B3
{
  Push2Words("NULL");
}


// ================================================
// 0xa4b7: WORD ''F1' codep=0xa49a wordp=0xa4bf
// ================================================
// 0xa4bf: db 0xb3 0xa4 '  '

// ================================================
// 0xa4c1: WORD ''F2' codep=0xa49a wordp=0xa4c9
// ================================================
// 0xa4c9: db 0xc8 0xce '  '

// ================================================
// 0xa4cb: WORD ''F3' codep=0xa49a wordp=0xa4d3
// ================================================
// 0xa4d3: db 0xae 0xd0 '  '

// ================================================
// 0xa4d5: WORD ''F4' codep=0xa49a wordp=0xa4dd
// ================================================
// 0xa4dd: db 0xd7 0xd1 '  '

// ================================================
// 0xa4df: WORD ''F5' codep=0xa49a wordp=0xa4e7
// ================================================
// 0xa4e7: db 0xb3 0xa4 '  '

// ================================================
// 0xa4e9: WORD ''F6' codep=0xa49a wordp=0xa4f1
// ================================================
// 0xa4f1: db 0x84 0xbe '  '

// ================================================
// 0xa4f3: WORD ''F7' codep=0xa49a wordp=0xa4fb
// ================================================
// 0xa4fb: db 0xb3 0xa4 '  '

// ================================================
// 0xa4fd: WORD ''F8' codep=0xa49a wordp=0xa505
// ================================================
// 0xa505: db 0x54 0xc7 'T '

// ================================================
// 0xa507: WORD ''F9' codep=0xa49a wordp=0xa50f
// ================================================
// 0xa50f: db 0x5a 0xc8 'Z '

// ================================================
// 0xa511: WORD ''F10' codep=0xa49a wordp=0xa51a
// ================================================
// 0xa51a: db 0x28 0xc7 '( '

// ================================================
// 0xa51c: WORD 'WA51E' codep=0x224c wordp=0xa51e params=0 returns=2
// ================================================

void WA51E() // WA51E
{
  Push(0);
  Push(1);
  Push(pp_FQUIT); // FQUIT
  ON_2(); // ON_2
}


// ================================================
// 0xa528: WORD 'WA52A' codep=0x224c wordp=0xa52a params=0 returns=2
// ================================================

void WA52A() // WA52A
{
  Push(-1);
  Push(-1);
}


// ================================================
// 0xa530: WORD 'WA532' codep=0x224c wordp=0xa532 params=0 returns=2
// ================================================

void WA532() // WA532
{
  Push(-1);
  Push(0);
}


// ================================================
// 0xa538: WORD 'WA53A' codep=0x224c wordp=0xa53a params=0 returns=2
// ================================================

void WA53A() // WA53A
{
  Push(-1);
  Push(1);
}


// ================================================
// 0xa540: WORD 'WA542' codep=0x224c wordp=0xa542 params=0 returns=2
// ================================================

void WA542() // WA542
{
  Push2Words("NULL");
}


// ================================================
// 0xa546: WORD 'WA548' codep=0x224c wordp=0xa548 params=0 returns=2
// ================================================

void WA548() // WA548
{
  Push2Words("NULL");
  Push(pp_FTRIG); // FTRIG
  ON_2(); // ON_2
}


// ================================================
// 0xa550: WORD 'DOTRACE' codep=0x224c wordp=0xa55c
// ================================================

void DOTRACE() // DOTRACE
{
  CTINIT(); // CTINIT
  WUP(); // WUP
  _gt_BOTT(); // >BOTT
  MAXSPAC(); // MAXSPAC
  _1_dot_5_at_(); // 1.5@
  GetNEWSPA(); // @NEWSPA
  D_dash_(); // D-
  D_dot_(); // D.
  CDEPTH(); // CDEPTH
  Draw(); // .
  DEPTH(); // DEPTH
  Draw(); // .
  W9F5E(); // W9F5E
}


// ================================================
// 0xa578: WORD ''TRACE' codep=0x1d29 wordp=0xa583
// ================================================
// 0xa583: db 0xa8 0x49 ' I'

// ================================================
// 0xa585: WORD 'WA587' codep=0x224c wordp=0xa587
// ================================================

void WA587() // WA587
{
  Push(pp__i_TRACE); // 'TRACE
  GetEXECUTE(); // @EXECUTE
}


// ================================================
// 0xa58d: WORD '(XYSCAN' codep=0x4b3b wordp=0xa599
// ================================================

void _ro_XYSCAN() // (XYSCAN
{
  switch(Pop()) // (XYSCAN
  {
  case 327:
    WA45F(); // WA45F
    break;
  case 328:
    WA467(); // WA467
    break;
  case 329:
    WA46F(); // WA46F
    break;
  case 2:
    W9F68(); // W9F68
    break;
  case 331:
    WA477(); // WA477
    break;
  case 333:
    WA51E(); // WA51E
    break;
  case 20:
    WA587(); // WA587
    break;
  case 335:
    WA52A(); // WA52A
    break;
  case 336:
    WA532(); // WA532
    break;
  case 337:
    WA53A(); // WA53A
    break;
  case 0:
    WA542(); // WA542
    break;
  case 92:
    WA45F(); // WA45F
    break;
  case 126:
    WA467(); // WA467
    break;
  case 124:
    WA477(); // WA477
    break;
  case 500:
    WA51E(); // WA51E
    break;
  case 335:
    WA52A(); // WA52A
    break;
  case 96:
    WA532(); // WA532
    break;
  case 315:
    Func12("'F1");
    break;
  case 316:
    Func12("'F2");
    break;
  case 317:
    Func12("'F3");
    break;
  case 318:
    Func12("'F4");
    break;
  case 319:
    Func12("'F5");
    break;
  case 320:
    Func12("'F6");
    break;
  case 321:
    Func12("'F7");
    break;
  case 322:
    Func12("'F8");
    break;
  case 323:
    Func12("'F9");
    break;
  case 324:
    Func12("'F10");
    break;
  case 338:
    Push2Words("NULL");
    break;
  default:
    WA548(); // WA548
    break;

  }
}

// ================================================
// 0xa60d: WORD 'XYSCAN' codep=0x224c wordp=0xa618
// ================================================

void XYSCAN() // XYSCAN
{
  Push(pp_FQUIT); // FQUIT
  _099(); // 099
  Push(pp_FTRIG); // FTRIG
  _099(); // 099
  _i_KEY(); // 'KEY
  _ro_XYSCAN(); // (XYSCAN case
}


// ================================================
// 0xa626: WORD '?TRIG' codep=0x224c wordp=0xa630 params=0 returns=1
// ================================================

void IsTRIG() // ?TRIG
{
  Push(Read16(pp_FTRIG)); // FTRIG @
  Push(Read16(regsp)); // DUP
  if (Pop() == 0) return;
  Push(pp_FTRIG); // FTRIG
  _099(); // 099
}


// ================================================
// 0xa640: WORD '?QUIT' codep=0x224c wordp=0xa64a params=0 returns=1
// ================================================

void IsQUIT() // ?QUIT
{
  Push(Read16(pp_FQUIT)); // FQUIT @
  Push(Read16(regsp)); // DUP
  if (Pop() == 0) return;
  Push(pp_FQUIT); // FQUIT
  _099(); // 099
}


// ================================================
// 0xa65a: WORD 'Y/N' codep=0x224c wordp=0xa662
// ================================================

void Y_slash_N() // Y/N
{
  do
  {
    XYSCAN(); // XYSCAN
    SWAP(); // SWAP
    Pop(); // DROP
    if (Read16(regsp) != 0) Push(Read16(regsp)); // ?DUP
  } while(Pop() == 0);
  CLICK(); // CLICK
  _0_gt_(); // 0>
}


// ================================================
// 0xa674: WORD 'WA676' codep=0x224c wordp=0xa676 params=0 returns=0
// ================================================

void WA676() // WA676
{
  Push(0x005d);
  Push(pp_XBLT); // XBLT
  Store_2(); // !_2
  Push(0xfff5);
  Push(pp_YBLT); // YBLT
  _plus__ex__2(); // +!_2
}


// ================================================
// 0xa688: WORD 'WA68A' codep=0x224c wordp=0xa68a
// ================================================
// orphan

void WA68A() // WA68A
{
  Push(pp__i_BUTTON); // 'BUTTON
  GetEXECUTE(); // @EXECUTE
}


// ================================================
// 0xa690: WORD '.ABTN' codep=0x224c wordp=0xa69a
// ================================================

void DrawABTN() // .ABTN
{
  unsigned short int a;
  a = Pop(); // >R
  Push(0x003a);
  Push(pp_FILE_n_); // FILE#
  Store_2(); // !_2
  Push(pp_RECORD_n_); // RECORD#
  Store_2(); // !_2
  Push(0x005d);
  Push(0x0077 - a * 0x000b); // 0x0077 I 0x000b * -
  POS_dot_(); // POS.
  _gt_2FONT(); // >2FONT
  Push(Read16(pp_YBLT)); // YBLT @
  Push(0x005d);
  OVER(); // OVER
  Push(Pop() - 7); //  7 -
  Push(0x009c);
  GetColor(BLACK);
  POLY_dash_WI(); // POLY-WI
  GetColor(WHITE);
  StoreCOLOR(); // !COLOR
  LoadData(_1BTN); // from 'BUTTONS'
  Push(Pop() + a * 0x000c); //  R> 0x000c * +
  Push(0x000c);
  Exec("TYPE"); // call of word 0x2690 '(TYPE)'
  SET_dash_CUR(); // SET-CUR
}


// ================================================
// 0xa6ee: WORD 'WA6F0' codep=0x224c wordp=0xa6f0
// ================================================

void WA6F0() // WA6F0
{
  Push((Pop()==0x003c?1:0) & (Read16(pp_RECORD_n_)==1?1:0)); //  0x003c = RECORD# @ 1 = AND
  if (Pop() != 0)
  {
    GetColor(PINK);
  } else
  {
    GetColor(WHITE);
  }
  StoreCOLOR(); // !COLOR
}


// ================================================
// 0xa710: WORD '.BTN-TE' codep=0x224c wordp=0xa71c
// ================================================

void DrawBTN_dash_TE() // .BTN-TE
{
  unsigned short int i, imax;
  Push(0x003a);
  Push(pp_FILE_n_); // FILE#
  Store_2(); // !_2
  Push(Read16(regsp)); // DUP
  Push(pp_RECORD_n_); // RECORD#
  Store_2(); // !_2
  Push(pp_BTN_dash_REC); // BTN-REC
  Store_2(); // !_2
  GetColor(BLACK);
  WA250(); // WA250
  Push(0x005d);
  Push(0x0077);
  POS_dot_(); // POS.
  _gt_2FONT(); // >2FONT
  LoadData(_n_BTN); // from 'BUTTONS'

  i = 0;
  imax = (Read16(Pop())&0xFF) * 0x000c; //  C@ 0x000c *
  do // (DO)
  {
    Push(i); // I
    WA6F0(); // WA6F0
    LoadData(_1BTN); // from 'BUTTONS'
    Push(Pop() + i); //  I +
    Push(0x000c);
    Exec("TYPE"); // call of word 0x2690 '(TYPE)'
    WA676(); // WA676
    Push(0x000c);
    int step = Pop();
    i += step;
    if (((step>=0) && (i>=imax)) || ((step<0) && (i<=imax))) break;
  } while(1); // (+LOOP)

  SET_dash_CUR(); // SET-CUR
}


// ================================================
// 0xa76a: WORD 'NEW-BUT' codep=0x224c wordp=0xa776
// ================================================

void NEW_dash_BUT() // NEW-BUT
{
  Push(0x003a);
  Push(pp_FILE_n_); // FILE#
  Store_2(); // !_2
  Push(Read16(pp_BTN_dash_REC)); // BTN-REC @
  Push(pp_RECORD_n_); // RECORD#
  Store_2(); // !_2
  Push(Read16(pp_THIS_dash_BU)); // THIS-BU @
  SWAP(); // SWAP
  _dash_(); // -
  Push(Read16(regsp)); // DUP
  Push(0);
  LoadData(_n_BTN); // from 'BUTTONS'
  Push(Read16(Pop())&0xFF); //  C@
  WITHIN(); // WITHIN
  if (Pop() != 0)
  {
    GetColor(BLUE);
    OVER(); // OVER
    GetColor(BLACK);
    Push(Read16(pp_THIS_dash_BU)); // THIS-BU @
    DrawHIGHLI(); // .HIGHLI
    DrawHIGHLI(); // .HIGHLI
    Push(pp_THIS_dash_BU); // THIS-BU
    Store_2(); // !_2
    CLICK(); // CLICK
  } else
  {
    Pop(); // DROP
  }
  SET_dash_CUR(); // SET-CUR
}


// ================================================
// 0xa7ba: WORD 'CURSORS' codep=0x224c wordp=0xa7c6 params=0 returns=1
// ================================================

void CURSORS() // CURSORS
{
  Push(Read16(pp_CURSEG)); // CURSEG @
  Push(pp_BLTSEG); // BLTSEG
  Store_2(); // !_2
  Push(0);
}


// ================================================
// 0xa7d2: WORD 'BLD-CRS' codep=0x224c wordp=0xa7de
// ================================================

void BLD_dash_CRS() // BLD-CRS
{
  unsigned short int a, b;
  GetCOLOR(); // @COLOR
  a = Pop(); // >R
  Push(pp_XORMODE); // XORMODE
  ON_2(); // ON_2
  Push(Read16(pp_CRSCOLO)); // CRSCOLO @
  StoreCOLOR(); // !COLOR
  CURSORS(); // CURSORS
  Push(pp_ABLT); // ABLT
  Store_2(); // !_2
  Push((Pop() + 1) + Read16(pp_OCRS) * 4); //  1+ OCRS @ 4 * +
  b = Pop(); // >R
  Push(Read16(b)&0xFF); // I C@
  Push(pp_YBLT); // YBLT
  Store_2(); // !_2
  Push(Read16(b + 1)&0xFF); // I 1+ C@
  Push(pp_XBLT); // XBLT
  Store_2(); // !_2
  Push(Read16(b + 2)&0xFF); // I 2+ C@
  Push(pp_LBLT); // LBLT
  Store_2(); // !_2
  Push(Read16(b + 3)&0xFF); // R> 3 + C@
  Push(pp_WBLT); // WBLT
  Store_2(); // !_2
  BLT(); // BLT
  Push(a); // R>
  StoreCOLOR(); // !COLOR
}


// ================================================
// 0xa830: WORD 'SET-CRS' codep=0x224c wordp=0xa83c params=1 returns=1
// ================================================

void SET_dash_CRS() // SET-CRS
{
  Push(Read16(pp_NCRS)); // NCRS @
  OVER(); // OVER
  Push((Read16(Pop())&0xFF) + 1); //  C@ 1+
  Push(0);
  SWAP(); // SWAP
  WITHIN(); // WITHIN
  if (Pop() != 0)
  {
    Push(!(Read16(pp_NCRS)==Read16(pp_OCRS)?1:0)); // NCRS @ OCRS @ = NOT
    if (Pop() != 0)
    {
      Push(Read16(regsp)); // DUP
      BLD_dash_CRS(); // BLD-CRS
      Push(Read16(pp_NCRS)); // NCRS @
      Push(pp_OCRS); // OCRS
      Store_2(); // !_2
      BLD_dash_CRS(); // BLD-CRS
      Push(1);
    } else
    {
      Pop(); // DROP
      Push(0);
    }
    return;
  }
  Push(Read16(Pop())&0xFF); //  C@
  Push(Read16(pp_OCRS)); // OCRS @
  MIN(); // MIN
  Push(0);
  MAX(); // MAX
  Push(Read16(regsp)); // DUP
  Push(pp_OCRS); // OCRS
  Store_2(); // !_2
  Push(pp_NCRS); // NCRS
  Store_2(); // !_2
  Push(0);
}


// ================================================
// 0xa896: WORD '$!' codep=0x224c wordp=0xa89d params=2 returns=0
// ================================================

void _do__ex_() // $!
{
  OVER(); // OVER
  Push((Read16(Pop())&0xFF) + 1); //  C@ 1+
  CMOVE_2(); // CMOVE_2
}


// ================================================
// 0xa8a7: WORD 'SUBROOT' codep=0x224c wordp=0xa8b3 params=0 returns=0
// ================================================

void SUBROOT() // SUBROOT
{
  CDEPTH(); // CDEPTH
  Push(pp_W5E52); // W5E52
  Store_2(); // !_2
}


// ================================================
// 0xa8bb: WORD 'SRDEPTH' codep=0x224c wordp=0xa8c7 params=0 returns=1
// ================================================

void SRDEPTH() // SRDEPTH
{
  CDEPTH(); // CDEPTH
  Push(Pop() - Read16(pp_W5E52)); //  W5E52 @ -
}


// ================================================
// 0xa8d1: WORD 'WA8D3' codep=0x224c wordp=0xa8d3 params=0 returns=0
// ================================================

void WA8D3() // WA8D3
{
  while(1)
  {
    IsLAST(); // ?LAST
    SRDEPTH(); // SRDEPTH
    _0_gt_(); // 0>
    Push(Pop() & Pop()); // AND
    if (Pop() == 0) return;
    ICLOSE(); // ICLOSE
  }
}


// ================================================
// 0xa8e7: WORD 'NEXT-NO' codep=0x224c wordp=0xa8f3 params=0 returns=0
// ================================================

void NEXT_dash_NO() // NEXT-NO
{
  W7512(); // W7512
  Push(Pop() | Pop()); // OR
  if (Pop() != 0)
  {
    IOPEN(); // IOPEN
    return;
  }
  WA8D3(); // WA8D3
  SRDEPTH(); // SRDEPTH
  _0_gt_(); // 0>
  if (Pop() == 0) return;
  INEXT(); // INEXT
}


// ================================================
// 0xa90f: WORD 'INST-VA' codep=0x7420 wordp=0xa91b
// ================================================
IFieldType INST_dash_VA = {DIRECTORYIDX, 0x11, 0x02};

// ================================================
// 0xa91e: WORD 'INST-DA' codep=0x7420 wordp=0xa92a
// ================================================
IFieldType INST_dash_DA = {DIRECTORYIDX, 0x13, 0x02};

// ================================================
// 0xa92d: WORD '%NAME' codep=0x7420 wordp=0xa937
// ================================================
IFieldType _pe_NAME = {SHIPIDX, 0x34, 0x0f};

// ================================================
// 0xa93a: WORD 'ORIG-NA' codep=0x7394 wordp=0xa946
// ================================================
LoadDataType ORIG_dash_NA = {MESSAGEIDX, 0x00, 0x10, 0x00, 0x0000};

// ================================================
// 0xa94c: WORD 'ELEM-NA' codep=0x7394 wordp=0xa958
// ================================================
LoadDataType ELEM_dash_NA = {ELEMENTIDX, 0x00, 0x10, 0x17, 0x6ba9};

// ================================================
// 0xa95e: WORD 'ELEM-VA' codep=0x7394 wordp=0xa96a
// ================================================
LoadDataType ELEM_dash_VA = {ELEMENTIDX, 0x10, 0x02, 0x17, 0x6ba9};

// ================================================
// 0xa970: WORD 'ART-NAM' codep=0x7394 wordp=0xa97c
// ================================================
LoadDataType ART_dash_NAM = {ARTIFACTIDX, 0x00, 0x18, 0x20, 0x6bcd};

// ================================================
// 0xa982: WORD 'ART-VAL' codep=0x7394 wordp=0xa98e
// ================================================
LoadDataType ART_dash_VAL = {ARTIFACTIDX, 0x1b, 0x02, 0x20, 0x6bcd};

// ================================================
// 0xa994: WORD 'ART-VOL' codep=0x7394 wordp=0xa9a0
// ================================================
LoadDataType ART_dash_VOL = {ARTIFACTIDX, 0x19, 0x02, 0x20, 0x6bcd};

// ================================================
// 0xa9a6: WORD 'PHR-CNT' codep=0x7420 wordp=0xa9b2
// ================================================
IFieldType PHR_dash_CNT = {STRINGIDX, 0x0b, 0x01};

// ================================================
// 0xa9b5: WORD 'PHRASE' codep=0x7420 wordp=0xa9c0
// ================================================
IFieldType PHRASE = {STRINGIDX, 0x0c, 0xfe};

// ================================================
// 0xa9c3: WORD 'ASKING' codep=0x7420 wordp=0xa9ce
// ================================================
IFieldType ASKING = {DIRECTORYIDX, 0x13, 0x02};

// ================================================
// 0xa9d1: WORD 'U>$' codep=0x224c wordp=0xa9d9
// ================================================

void U_gt__do_() // U>$
{
  Push(0);
  _st__n_(); // <#
  _n_S(); // #S
  _n__gt_(); // #>
}


// ================================================
// 0xa9e3: WORD 'WA9E5' codep=0x224c wordp=0xa9e5 params=0 returns=1
// ================================================

void WA9E5() // WA9E5
{
  unsigned short int i, imax;

  i = 0;
  imax = 0x000b;
  do // (DO)
  {
    Push(i); // I
    GetTableEntry("OVT");
    Push(Read16(Pop())); //  @
    i++;
  } while(i<imax); // (LOOP)

}


// ================================================
// 0xa9f9: WORD 'WA9FB' codep=0x224c wordp=0xa9fb params=1 returns=0
// ================================================

void WA9FB() // WA9FB
{
  unsigned short int i, imax;

  i = 0x000a;
  imax = 0;
  do // (DO)
  {
    Push(i); // I
    GetTableEntry("OVT");
    Store_2(); // !_2
    Push(-1);
    int step = Pop();
    i += step;
    if (((step>=0) && (i>=imax)) || ((step<0) && (i<=imax))) break;
  } while(1); // (+LOOP)

}


// ================================================
// 0xaa11: WORD '<CTVERS' codep=0x224c wordp=0xaa1d
// ================================================

void _st_CTVERS() // <CTVERS
{
  unsigned short int a, b;
  a = Pop(); // >R
  b = Pop(); // >R
  WA9E5(); // WA9E5
  Push(b); // R>
  Push(a); // R>
  CTINIT(); // CTINIT
  W7021(); // W7021
  WA9FB(); // WA9FB
}


// ================================================
// 0xaa2f: WORD '<CTASKM' codep=0x224c wordp=0xaa3b params=0 returns=0
// ================================================

void _st_CTASKM() // <CTASKM
{
  WA9E5(); // WA9E5
  Push(Read16(pp_FONT_n_)); // FONT# @
  if (Pop() != 0)
  {
    CTINIT(); // CTINIT
  }
  _st_ASKMOU(); // <ASKMOU
  WA9FB(); // WA9FB
}


// ================================================
// 0xaa4d: WORD 'TEXT-CO' codep=0x7420 wordp=0xaa59
// ================================================
IFieldType TEXT_dash_CO = {SCROLL_TEXTIDX, 0x0b, 0x03};

// ================================================
// 0xaa5c: WORD 'TEXT-IN' codep=0x7420 wordp=0xaa68
// ================================================
IFieldType TEXT_dash_IN = {SCROLL_TEXTIDX, 0x0e, 0x03};

// ================================================
// 0xaa6b: WORD 'TEXT-TE' codep=0x7420 wordp=0xaa77
// ================================================
IFieldType TEXT_dash_TE = {SCROLL_TEXTIDX, 0x11, 0x26};

// ================================================
// 0xaa7a: WORD 'TEXT>PA' codep=0x224c wordp=0xaa86 params=0 returns=0
// ================================================

void TEXT_gt_PA() // TEXT>PA
{
  Push(0x65e1+TEXT_dash_TE.offset); // TEXT-TE<IFIELD>
  PAD(); // PAD
  Push(Pop() + 1); //  1+
  Push(0x0026);
  CMOVE_2(); // CMOVE_2
  Push(0x0026);
  PAD(); // PAD
  C_ex__2(); // C!_2
}


// ================================================
// 0xaa9c: WORD 'CMESS' codep=0x224c wordp=0xaaa6
// ================================================

void CMESS() // CMESS
{
  unsigned short int a, b;
  a = Pop(); // >R
  b = Pop(); // >R
  GetCRS(); // @CRS
  Push(b); // R>
  Push(a); // R>
  CTINIT(); // CTINIT
  Push(Pop() * 2); //  2*
  Push(0x0050);
  SWAP(); // SWAP
  _dash_(); // -
  Push(pp_XBLT); // XBLT
  Store_2(); // !_2
  Push(0x0017);
  SWAP(); // SWAP
  Push(Pop() * 6); //  6 *
  _dash_(); // -
  Push(pp_YBLT); // YBLT
  Store_2(); // !_2
}


// ================================================
// 0xaad2: WORD 'X0MESS' codep=0x224c wordp=0xaadd
// ================================================

void X0MESS() // X0MESS
{
  Push(0x0017);
  ROT(); // ROT
  Push(Pop() * 6); //  6 *
  _dash_(); // -
  Push(0x0026);
  ROT(); // ROT
  ROT(); // ROT
  Push(2);
  Push(Read16(pp_CONTEXT_3)); // CONTEXT_3 @
  Push(5);
  _st_(); // <
  if (Pop() != 0)
  {
    Push(4);
    MAX(); // MAX
  }
  POS_dot_PXT(); // POS.PXT
}


// ================================================
// 0xab07: WORD '0MESS' codep=0x224c wordp=0xab11
// ================================================

void _0MESS() // 0MESS
{
  GetColor(BLACK);
  X0MESS(); // X0MESS
}


// ================================================
// 0xab17: WORD 'WAB19' codep=0x2214 wordp=0xab19
// ================================================
// 0xab19: dw 0x002e

// ================================================
// 0xab1b: WORD '-XTRAIL' codep=0x224c wordp=0xab27 params=3 returns=2
// ================================================

void _dash_XTRAIL() // -XTRAIL
{
  Push(0xab19); // 'WAB19'
  Store_2(); // !_2
  Push(0xab19); // 'WAB19'
  CFA(); // CFA
  Push(0x2790); // '-TRAILING'
  Push(Pop() + 0x000e); //  0x000e +
  Store_2(); // !_2
  _dash_TRAILING(); // -TRAILING
  Push(0x099a); // 'BL'
  CFA(); // CFA
  Push(0x2790); // '-TRAILING'
  Push(Pop() + 0x000e); //  0x000e +
  Store_2(); // !_2
}


// ================================================
// 0xab55: WORD 'WAB57' codep=0x1d29 wordp=0xab57
// ================================================
// 0xab57: db 0x06 0x00 '  '

// ================================================
// 0xab59: WORD 'SMART' codep=0x1d29 wordp=0xab63
// ================================================
// 0xab63: db 0x00 0x00 '  '

// ================================================
// 0xab65: WORD 'WAB67' codep=0x2214 wordp=0xab67
// ================================================
// 0xab67: dw 0x00ff

// ================================================
// 0xab69: WORD 'WAB6B' codep=0x2214 wordp=0xab6b
// ================================================
// 0xab6b: dw 0xec67

// ================================================
// 0xab6d: WORD 'WAB6F' codep=0x2214 wordp=0xab6f
// ================================================
// 0xab6f: dw 0xec68

// ================================================
// 0xab71: WORD 'WAB73' codep=0x2214 wordp=0xab73
// ================================================
// 0xab73: dw 0xec69

// ================================================
// 0xab75: WORD 'WAB77' codep=0x2214 wordp=0xab77
// ================================================
// 0xab77: dw 0xec6a

// ================================================
// 0xab79: WORD 'WAB7B' codep=0x2214 wordp=0xab7b
// ================================================
// 0xab7b: dw 0xec78

// ================================================
// 0xab7d: WORD 'WAB7F' codep=0x2214 wordp=0xab7f
// ================================================
// 0xab7f: dw 0xec84

// ================================================
// 0xab81: WORD 'WAB83' codep=0x224c wordp=0xab83 params=1 returns=1
// ================================================

void WAB83() // WAB83
{
  Push((Read16(Pop())&0xFF) & 0x007f); //  C@ 0x007f AND
}


// ================================================
// 0xab8d: WORD 'C+!' codep=0x224c wordp=0xab95 params=2 returns=0
// ================================================

void C_plus__ex_() // C+!
{
  Push(Read16(regsp)); // DUP
  Push(Read16(Pop())&0xFF); //  C@
  ROT(); // ROT
  Push(Pop() + Pop()); // +
  SWAP(); // SWAP
  C_ex__2(); // C!_2
}


// ================================================
// 0xaba3: WORD 'WABA5' codep=0x224c wordp=0xaba5 params=1 returns=0
// ================================================

void WABA5() // WABA5
{
  unsigned short int a;
  a = Pop(); // >R
  Push(a); // I
  Push(0xab6b); // 'WAB6B'
  Store_2(); // !_2
  Push(a + 1); // I 1+
  Push(0xab6f); // 'WAB6F'
  Store_2(); // !_2
  Push(a + 2); // I 2+
  Push(0xab73); // 'WAB73'
  Store_2(); // !_2
  Push(a + 3); // R> 3 +
  Push(0xab77); // 'WAB77'
  Store_2(); // !_2
  Push((Read16(Read16(cc_WAB6B))&0xFF) * 2 + Read16(cc_WAB77)); // WAB6B C@ 2* WAB77 +
  Push(0xab7b); // 'WAB7B'
  Store_2(); // !_2
  Push((Read16(Read16(cc_WAB6F))&0xFF) * 2 + Read16(cc_WAB7B)); // WAB6F C@ 2* WAB7B +
  Push(0xab7f); // 'WAB7F'
  Store_2(); // !_2
}


// ================================================
// 0xabf1: WORD 'DISTRAC' codep=0x224c wordp=0xabfd params=1 returns=0
// ================================================

void DISTRAC() // DISTRAC
{
  WABA5(); // WABA5
  Push(Read16(cc_WAB7F)); // WAB7F
  Push(Read16(Read16(cc_WAB6F))&0xFF); // WAB6F C@
  Push(Read16(cc_WAB67)); // WAB67
  FILL_2(); // FILL_2
}


// ================================================
// 0xac0b: WORD 'WAC0D' codep=0x224c wordp=0xac0d params=1 returns=1
// ================================================

void WAC0D() // WAC0D
{
  unsigned short int a, i, imax, b;
  a = Pop(); // >R
  Push(Read16(cc_TRUE)); // TRUE
  Push(a + 3); // I 3 +
  Push(Read16(regsp)); // DUP
  Push(Pop() + (Read16(a)&0xFF)); //  I C@ +
  SWAP(); // SWAP

  i = Pop();
  imax = Pop();
  do // (DO)
  {
    Push(Read16(cc_WAB7F)); // WAB7F
    Push(i); // I
    WAB83(); // WAB83
    Push(Pop() + Pop()); // +
    Push(Read16(Pop())&0xFF); //  C@
    Push(Read16(regsp)); // DUP
    Push(Pop()==Read16(cc_WAB67)?1:0); //  WAB67 =
    if (Pop() != 0)
    {
      Pop(); // DROP
      Push(Read16(cc_WAB7B)); // WAB7B
      Push(i); // I
      WAB83(); // WAB83
      Push(Pop() * 2); //  2*
      Push(Pop() + Pop()); // +
      GetEXECUTE(); // @EXECUTE
      Push(!(Pop()==0?1:0)); //  0= NOT
      Push(Read16(regsp)); // DUP
      Push(Read16(cc_WAB7F)); // WAB7F
      Push(i); // I
      WAB83(); // WAB83
      Push(Pop() + Pop()); // +
      C_ex__2(); // C!_2
    }
    Push(Pop()==!(!((Read16(i)&0xFF) & 0x0080))?1:0); //  I C@ 0x0080 AND NOT NOT =
    Push(Pop() & Pop()); // AND
    Push(Read16(regsp)); // DUP
    Push(Pop()==0?1:0); //  0=
    if (Pop() != 0)
    {
      imax = i; // LEAVE
    }
    Push(1);
    i += Pop();
  } while(i<imax); // (/LOOP)

  Push(Read16(regsp)); // DUP
  if (Pop() != 0)
  {
    b = Pop(); // >R
    Push(a + 1); // I' 1+
    GetEXECUTE(); // @EXECUTE
    Push(b); // R>
  }
  Push(a); // R>
  Pop(); // DROP
}


// ================================================
// 0xac8d: WORD 'EXPERT' codep=0x224c wordp=0xac98
// ================================================

void EXPERT() // EXPERT
{
  unsigned short int a, i, imax;
  Push(pp_WAB57); // WAB57
  _099(); // 099
  Push(Read16(pp_TRACE)); // TRACE @
  if (Pop() != 0)
  {
    _gt_0FONT_2(); // >0FONT_2
  }
  Exec("CREATE"); // call of word 0x1cbb '(CREATE)'
  HERE(); // HERE
  a = Pop(); // >R
  C_co_(); // C,
  C_co_(); // C,
  Push(0);
  C_co_(); // C,
  Push(a); // R>
  WABA5(); // WABA5
  Push((Read16(Read16(cc_WAB6B))&0xFF) * 2 + (Read16(Read16(cc_WAB6F))&0xFF) * 3); // WAB6B C@ 2* WAB6F C@ 3 * +
  ALLOT(); // ALLOT
  CODE(); // (;CODE) inlined assembler code
// 0xaccc: call   1649
  Push(Read16(regsp)); // DUP
  WABA5(); // WABA5
  _gt_V(); // >V
  Push(Read16(cc_WAB77)); // WAB77
  Push(Read16(regsp)); // DUP
  Push(Pop() + (Read16(Read16(cc_WAB73))&0xFF) * 2); //  WAB73 C@ 2* +
  SWAP(); // SWAP

  i = Pop();
  imax = Pop();
  do // (DO)
  {
    Push(Read16(i)); // I @
    WAC0D(); // WAC0D
    if (Pop() != 0)
    {
      V_gt_(); // V>
      Push(Read16(regsp)); // DUP
      WABA5(); // WABA5
      Push(!Pop()); //  NOT
      _gt_V(); // >V
      imax = i; // LEAVE
      Push(Read16(pp_SMART)); // SMART @
      if (Pop() != 0)
      {
        Push(((i - Read16(cc_WAB77) >> 1) >> 1) * 2 + Read16(cc_WAB77)); // I WAB77 - 2/ 2/ 2* WAB77 +
        Push(Read16(regsp)); // DUP
        Push(Read16(Pop())); //  @
        Push(Read16(i)); // I @
        SWAP(); // SWAP
        Push(i); // I
        Store_2(); // !_2
        SWAP(); // SWAP
        Store_2(); // !_2
      }
    }
    Push(2);
    i += Pop();
  } while(i<imax); // (/LOOP)

  V_gt_(); // V>
  Push(!Pop()); //  NOT
}


// ================================================
// 0xad31: WORD '-->' codep=0x224c wordp=0xad39
// ================================================

void _dash__dash__gt_() // -->
{
  _i__2(); // '_2
}


// ================================================
// 0xad3d: WORD 'WAD3F' codep=0x224c wordp=0xad3f params=1 returns=1
// ================================================

void WAD3F() // WAD3F
{
  unsigned short int a, i, imax;
  a = Pop(); // >R
  Push(-1);

  i = 0;
  imax = Read16(pp_WAB57) * 2; // WAB57 @ 2*
  do // (DO)
  {
    Push(Read16(Read16(cc_WAB7B) + i)==a?1:0); // WAB7B I + @ J =
    if (Pop() != 0)
    {
      Pop(); // DROP
      Push(i >> 1); // I 2/
      imax = i; // LEAVE
    }
    Push(2);
    i += Pop();
  } while(i<imax); // (/LOOP)

  Push(a); // R>
  Pop(); // DROP
}


// ================================================
// 0xad71: WORD 'WAD73' codep=0x224c wordp=0xad73
// ================================================

void WAD73() // WAD73
{
  unsigned short int a;
  _dash__dash__gt_(); // -->
  a = Pop(); // >R
  Push(a); // I
  Push(0xad39); // '-->'
  Push((Pop()==Pop())?1:0); // =
  Push(!Pop()); //  NOT
  if (Pop() != 0)
  {
    Push(a); // I
    WAD3F(); // WAD3F
    Push(Read16(regsp)); // DUP
    _0_st_(); // 0<
    if (Pop() != 0)
    {
      Pop(); // DROP
      Push(Read16(pp_WAB57)); // WAB57 @
      Push(a); // I
      OVER(); // OVER
      Push(Pop() * 2 + Read16(cc_WAB7B)); //  2* WAB7B +
      Store_2(); // !_2
      Push(Read16(Read16(cc_WAB6F))&0xFF); // WAB6F C@
      Push(Read16(pp_WAB57)); // WAB57 @
      _gt_(); // >
      Push(!Pop()); //  NOT
      ABORT("Condition overflow", 18);// (ABORT")
      Push(1);
      Push(pp_WAB57); // WAB57
      _plus__ex__2(); // +!_2
    }
  } else
  {
    Push(-1);
  }
  Push(a); // R>
  Pop(); // DROP
}


// ================================================
// 0xadd6: WORD 'RULE:' codep=0x224c wordp=0xade0
// ================================================

void RULE_c_() // RULE:
{
  Push(Read16(Read16(cc_WAB6B))&0xFF); // WAB6B C@
  Push(Read16(Read16(cc_WAB73))&0xFF); // WAB73 C@
  _gt_(); // >
  Push(!Pop()); //  NOT
  ABORT("Rule overflow", 13);// (ABORT")
  HERE(); // HERE
  Push(0);
  C_co_(); // C,
  Push(0x3a48); // 'NOP'
  _co_(); // ,
  do
  {
    WAD73(); // WAD73
    Push(Read16(regsp)); // DUP
    _0_st_(); // 0<
    Push(!Pop()); //  NOT
    if (Pop() != 0)
    {
      _i__2(); // '_2
      Push(Read16(regsp)); // DUP
      Push(0x51c5); // 'TRUE'
      Push((Pop()==Pop())?1:0); // =
      OVER(); // OVER
      Push(0x51d1); // 'FALSE'
      Push((Pop()==Pop())?1:0); // =
      Push(Pop() | Pop()); // OR
      Push(!Pop()); //  NOT
      ABORT("TRUE or FALSE needed", 20);// (ABORT")
      EXECUTE(); // EXECUTE
      Push(Pop() * 0x0080); //  0x0080 *
      Push(Pop() + Pop()); // +
      C_co_(); // C,
      Push(1);
      OVER(); // OVER
      C_plus__ex_(); // C+!
      Push(0);
    }
  } while(Pop() == 0);
  _dash__dash__gt_(); // -->
  OVER(); // OVER
  Push(Pop() + 1); //  1+
  Store_2(); // !_2
  Push((Read16(Read16(cc_WAB73))&0xFF) * 2 + Read16(cc_WAB77)); // WAB73 C@ 2* WAB77 +
  Store_2(); // !_2
  Push(1);
  Push(Read16(cc_WAB73)); // WAB73
  C_plus__ex_(); // C+!
}


// ================================================
// 0xae75: WORD 'CEX+WAX' codep=0x224c wordp=0xae81
// ================================================

void CEX_plus_WAX() // CEX+WAX
{
  Push(pp__i_CEX_plus_); // 'CEX+
  GetEXECUTE(); // @EXECUTE
  Push(pp__i_WAX); // 'WAX
  GetEXECUTE(); // @EXECUTE
}


// ================================================
// 0xae8b: WORD 'WAE8D' codep=0x224c wordp=0xae8d
// ================================================

void WAE8D() // WAE8D
{
  Push(pp__i_EXTERN); // 'EXTERN
  GetEXECUTE(); // @EXECUTE
}


// ================================================
// 0xae93: WORD 'WAE95' codep=0x224c wordp=0xae95
// ================================================

void WAE95() // WAE95
{
  Push(pp__i_REPAIR); // 'REPAIR
  GetEXECUTE(); // @EXECUTE
}


// ================================================
// 0xae9b: WORD 'WAE9D' codep=0x224c wordp=0xae9d
// ================================================

void WAE9D() // WAE9D
{
  Push(pp__i_VEHICL); // 'VEHICL
  GetEXECUTE(); // @EXECUTE
}


// ================================================
// 0xaea3: WORD 'WAEA5' codep=0x224c wordp=0xaea5
// ================================================

void WAEA5() // WAEA5
{
  Push(pp__i__dot_VEHIC); // '.VEHIC
  GetEXECUTE(); // @EXECUTE
}


// ================================================
// 0xaeab: WORD 'WAEAD' codep=0x224c wordp=0xaead
// ================================================

void WAEAD() // WAEAD
{
  WAE9D(); // WAE9D
  if (Pop() == 0) return;
  WAE95(); // WAE95
  WAEA5(); // WAEA5
}


// ================================================
// 0xaeb9: WORD 'WAEBB' codep=0x224c wordp=0xaebb
// ================================================

void WAEBB() // WAEBB
{
  Push(pp__i_TREATM); // 'TREATM
  GetEXECUTE(); // @EXECUTE
}


// ================================================
// 0xaec1: WORD 'WAEC3' codep=0x224c wordp=0xaec3
// ================================================

void WAEC3() // WAEC3
{
  Push(pp__i__dot_VITAL); // '.VITAL
  GetEXECUTE(); // @EXECUTE
}


// ================================================
// 0xaec9: WORD 'WAECB' codep=0x224c wordp=0xaecb
// ================================================

void WAECB() // WAECB
{
  Push(pp__i_CREW_dash_C); // 'CREW-C
  GetEXECUTE(); // @EXECUTE
}


// ================================================
// 0xaed1: WORD 'WAED3' codep=0x224c wordp=0xaed3
// ================================================

void WAED3() // WAED3
{
  WAECB(); // WAECB
  if (Pop() == 0) return;
  WAEBB(); // WAEBB
  WAEC3(); // WAEC3
}


// ================================================
// 0xaedf: WORD '?NEW-HO' codep=0x224c wordp=0xaeeb params=0 returns=1
// ================================================

void IsNEW_dash_HO() // ?NEW-HO
{
  TIME(); // TIME
  _2_at_(); // 2@
  Push(pp_LAST_dash_UP); // LAST-UP
  _2_at_(); // 2@
  D_dash_(); // D-
  DABS(); // DABS
  Push(pp_REAL_dash_MS); // REAL-MS
  _2_at_(); // 2@
  D_gt_(); // D>
}


// ================================================
// 0xaeff: WORD 'WAF01' codep=0x224c wordp=0xaf01
// ================================================

void WAF01() // WAF01
{
  Push(pp__i__dot_DATE); // '.DATE
  GetEXECUTE(); // @EXECUTE
}


// ================================================
// 0xaf07: WORD 'WAF09' codep=0x224c wordp=0xaf09 params=0 returns=0
// ================================================

void WAF09() // WAF09
{
  IsNEW_dash_HO(); // ?NEW-HO
  if (Pop() == 0) return;
  TIME(); // TIME
  _2_at_(); // 2@
  Push(pp_LAST_dash_UP); // LAST-UP
  StoreD(); // D!
  Push(Read16(pp_IsSECURE)); // ?SECURE @
  if (Read16(regsp) != 0) Push(Read16(regsp)); // ?DUP
  if (Pop() != 0)
  {
    Push(Read16(pp_STARDAT)); // STARDAT @
    _st_(); // <
    Push(Read16(pp_CONTEXT_3)); // CONTEXT_3 @
    Push(2);
    Push(4);
    WITHIN(); // WITHIN
    Push(Pop() & Pop()); // AND
    if (Pop() != 0)
    {
      Push(Read16(pp__i_STP)); // 'STP @
      MODULE(); // MODULE
    }
  }
  Push(Read16(cc_TRUE)); // TRUE
  if (Pop() != 0)
  {
    Push(Read16(pp_STAR_dash_HR)==0x0017?1:0); // STAR-HR @ 0x0017 =
    if (Pop() != 0)
    {
      Push(1);
      Push(pp_STARDAT); // STARDAT
      _plus__ex__2(); // +!_2
      Push(pp_STAR_dash_HR); // STAR-HR
      _099(); // 099
    } else
    {
      Push(1);
      Push(pp_STAR_dash_HR); // STAR-HR
      _plus__ex__2(); // +!_2
    }
  }
  WAF01(); // WAF01
  Push(Read16(pp_IsSUP)); // ?SUP @
  if (Pop() == 0) return;
  Push(pp__i_ENERGY); // 'ENERGY
  GetEXECUTE(); // @EXECUTE
}


// ================================================
// 0xaf75: WORD 'PARALLE' codep=0x224c wordp=0xaf81 params=0 returns=0
// ================================================

void PARALLE() // PARALLE
{
  Push(Read16(pp_TIME_dash_PA)); // TIME-PA @
  if (Pop() == 0) return;
  WAF09(); // WAF09
  WAED3(); // WAED3
  WAEAD(); // WAEAD
  WAE8D(); // WAE8D
}


// ================================================
// 0xaf93: WORD 'WAF95' codep=0x224c wordp=0xaf95 params=0 returns=0
// ================================================

void WAF95() // WAF95
{
  Push(Read16(pp__i_CLEANU)); // 'CLEANU @
  _gt_V(); // >V
  Push(Read16(pp__i_KEY_dash_CA)); // 'KEY-CA @
  _gt_V(); // >V
}


// ================================================
// 0xafa3: WORD 'WAFA5' codep=0x224c wordp=0xafa5 params=0 returns=0
// ================================================

void WAFA5() // WAFA5
{
  V_gt_(); // V>
  Push(pp__i_KEY_dash_CA); // 'KEY-CA
  Store_2(); // !_2
  V_gt_(); // V>
  Push(pp__i_CLEANU); // 'CLEANU
  Store_2(); // !_2
}


// ================================================
// 0xafb3: WORD 'WAFB5' codep=0x224c wordp=0xafb5
// ================================================

void WAFB5() // WAFB5
{
  Push(pp__i_KEY_dash_CA); // 'KEY-CA
  GetEXECUTE(); // @EXECUTE
}


// ================================================
// 0xafbb: WORD 'DOTASKS' codep=0x224c wordp=0xafc7
// ================================================

void DOTASKS() // DOTASKS
{
  WAF95(); // WAF95
  EXECUTE(); // EXECUTE
  Push(pp__i_KEY_dash_CA); // 'KEY-CA
  Store_2(); // !_2
  Push(pp__i_CLEANU); // 'CLEANU
  Store_2(); // !_2
  Push(1);
  do
  {
    Exec("IsTERMINAL"); // call of word 0x25bc '(?TERMINAL)'
    Push(Pop() | Read16(pp_FORCEKE)); //  FORCEKE @ OR
    if (Pop() != 0)
    {
      _i_KEY(); // 'KEY
      WAFB5(); // WAFB5
      TIME(); // TIME
      _2_at_(); // 2@
      Push(pp_KEYTIME); // KEYTIME
      StoreD(); // D!
    } else
    {
      Push(pp_SKEY); // SKEY
      _099(); // 099
      Push(pp_LKEY); // LKEY
      _099(); // 099
    }
    TIME(); // TIME
    _2_at_(); // 2@
    Push(pp_KEYTIME); // KEYTIME
    _2_at_(); // 2@
    D_dash_(); // D-
    Push(0x01f4); Push(0x0000);
    D_gt_(); // D>
    Push(Pop() | Read16(pp_FORCEPT)); //  FORCEPT @ OR
    if (Pop() != 0)
    {
      PARALLE(); // PARALLE
    }
    if (Read16(regsp) != 0) Push(Read16(regsp)); // ?DUP
    Push(Pop()==0?1:0); //  0=
  } while(Pop() == 0);
  Push(pp__i_CLEANU); // 'CLEANU
  GetEXECUTE(); // @EXECUTE
  WAFA5(); // WAFA5
}


// ================================================
// 0xb027: WORD '?VCYCLE' codep=0x224c wordp=0xb033 params=0 returns=1
// ================================================

void IsVCYCLE() // ?VCYCLE
{
  TIME(); // TIME
  _2_at_(); // 2@
  Push(pp_VSTIME); // VSTIME
  _2_at_(); // 2@
  D_gt_(); // D>
}

// 0xb03f: db 0x41 0xb0 0x80 0xe4 0x80 0x80 0xfc 0x00 0x74 0x11 0x1e 0x55 0x1f 0x8b 0x44 0x04 0x1f 0x25 0x00 0x80 0x0b 0xc0 0x75 0x03 0xbf 0x0d 0x00 0x8b 0xc7 0x2e 0x88 0x06 0xa2 0x61 0xb0 0x03 0xcf 'A       t  U  D  %    u      .   a   '

// ================================================
// 0xb064: WORD 'WB066' codep=0x224c wordp=0xb066 params=0 returns=0
// ================================================

void WB066() // WB066
{
  Push(0x0024);
  _ro__ex_OLD_rc_(); // (!OLD)
  Push(pp_W6370); // W6370
  _2_ex__2(); // 2!_2
  GetDS(); // @DS
  Push(0xb041);
  Push(0x0024);
  _ro__ex_SET_rc_(); // (!SET)
}


// ================================================
// 0xb07e: WORD 'WB080' codep=0x224c wordp=0xb080 params=0 returns=0
// ================================================

void WB080() // WB080
{
  Push(pp_W6370); // W6370
  _2_at_(); // 2@
  Push(0x0024);
  _ro__ex_SET_rc_(); // (!SET)
}


// ================================================
// 0xb08c: WORD '>XOR' codep=0x224c wordp=0xb095 params=3 returns=0
// ================================================

void _gt_XOR() // >XOR
{
  unsigned short int a, i, imax;
  ROT(); // ROT
  a = Pop(); // >R
  OVER(); // OVER
  Push(Pop() + Pop()); // +
  SWAP(); // SWAP

  i = Pop();
  imax = Pop();
  do // (DO)
  {
    Push(a); // J
    Push(i); // I
    LC_at_(); // LC@
    Push(Pop() + 0x007f); //  0x007f XOR
    Push(a); // J
    Push(i); // I
    LC_ex_(); // LC!
    Push(1);
    i += Pop();
  } while(i<imax); // (/LOOP)

  Push(a); // R>
  Pop(); // DROP
}


// ================================================
// 0xb0bf: WORD 'LFCLAIM' codep=0x224c wordp=0xb0c1 params=1 returns=1
// ================================================

void LFCLAIM() // LFCLAIM
{
  Push(-Pop() + Read16(pp_LFSEG)); //  NEGATE LFSEG @ +
  Push(Read16(regsp)); // DUP
  Push(pp_LFSEG); // LFSEG
  Store_2(); // !_2
}


// ================================================
// 0xb0d1: WORD 'AINSTALL' codep=0x224c wordp=0xb0d3 params=1 returns=0
// ================================================

void AINSTALL() // AINSTALL
{
  Push(Read16(regsp)); // DUP
  Push(pp__i_ARRAY); // 'ARRAY
  Store_2(); // !_2
  _n_BYTES(); // #BYTES
  _n_ROWZ(); // #ROWZ
  Push(Pop() * 2); //  2*
  Push(Pop() + Pop()); // +
  Push((Pop() >> 4) + 1); //  16/ 1+
  LFCLAIM(); // LFCLAIM
  OVER(); // OVER
  Push(Pop() + 6); //  6 +
  Store_2(); // !_2
  StoreOFFSETS(); // !OFFSETS
}


// ================================================
// 0xb0f3: WORD 'BINSTALL' codep=0x224c wordp=0xb0f5 params=2 returns=0
// ================================================

void BINSTALL() // BINSTALL
{
  LFCLAIM(); // LFCLAIM
  SWAP(); // SWAP
  Store_2(); // !_2
}


// ================================================
// 0xb0fd: WORD 'AINSTALLS' codep=0x224c wordp=0xb0ff params=0 returns=0
// ================================================

void AINSTALLS() // AINSTALLS
{
  unsigned short int i, imax;

  i = 0;
  imax = 0x000f;
  do // (DO)
  {
    Push(i); // I
    GetTableEntry("IARRAYS");
    AINSTALL(); // AINSTALL
    i++;
  } while(i<imax); // (LOOP)

  Push(0x6ade); // 'PPOLY'
  Push(Read16(Pop() + 6)); //  6 + @
  Push(0x52a2); // 'POLYSEG'
  Store_2(); // !_2
}


// ================================================
// 0xb123: WORD 'WB125' codep=0x224c wordp=0xb125 params=0 returns=8
// ================================================

void WB125() // WB125
{
  Push(pp_ICON_h_); // ICON^
  Push(0x00c9);
  Push(pp_PAL_h_); // PAL^
  Push(0x0028);
  Push(pp_SIL_h_); // SIL^
  Push(0x00ce);
  Push(pp_PIC_h_); // PIC^
  Push(Read16(cc__16K) >> 4); // 16K 16/
}


// ================================================
// 0xb13f: WORD 'BINSTALLS' codep=0x224c wordp=0xb141 params=0 returns=0
// ================================================

void BINSTALLS() // BINSTALLS
{
  unsigned short int i, imax;
  WB125(); // WB125
  Push(pp_XBUF_dash_SE); // XBUF-SE
  Push2Words("DBUF-SI");
  Push(0x0010);
  M_slash_(); // M/
  SWAP(); // SWAP
  Pop(); // DROP
  Push(pp_FONTSEG); // FONTSEG
  Push(0x0591);
  Push(0x516c); // 'MUSSEG'
  Push(0x0032);
  Push(pp_DICT); // DICT
  Push(0x0b63);
  Push(pp_SSYSEG); // SSYSEG
  Push(0x007a);
  Push(pp_MSYSEG); // MSYSEG
  Push(0x007a);
  Push(pp_LSYSEG); // LSYSEG
  Push(0x007a);
  Push(pp_CURSEG); // CURSEG
  Push(0x000d);
  Push(pp_IHSEG); // IHSEG
  Push(0x002d);
  Push(pp_W5B9E); // W5B9E
  Push(0x005a);
  Push(pp_W5B9A); // W5B9A
  Push(0x002d);
  Push(pp_W5B96); // W5B96
  Push(0x002d);
  Push(pp_W5B92); // W5B92
  Push(0x005a);
  Push(pp_W5B8E); // W5B8E
  Push(0x005a);
  Push(pp_W566E); // W566E
  Push(0x001f);
  Push(pp_W566A); // W566A
  Push(0x003e);
  Push(pp_W5672); // W5672
  Push(0x003e);
  Push(pp_W5676); // W5676
  Push(0x0136);

  i = 0;
  imax = 0x0016;
  do // (DO)
  {
    BINSTALL(); // BINSTALL
    i++;
  } while(i<imax); // (LOOP)

}


// ================================================
// 0xb1c9: WORD 'ICINIT' codep=0x224c wordp=0xb1d4 params=0 returns=0
// ================================================

void ICINIT() // ICINIT
{
  Push(0x01f0);
  Push(pp_W56DC); // W56DC
  Store_2(); // !_2
  Push(0x1360);
  Push(pp_W56E4); // W56E4
  Store_2(); // !_2
  Push(pp_W56E8); // W56E8
  _099(); // 099
  Push(pp_W56E0); // W56E0
  _099(); // 099
  Push2Words("NULL");
  Push(pp_W639C); // W639C
  _2_ex__2(); // 2!_2
  Push(pp_W55FF); // W55FF
  _099(); // 099
  Push(pp_IBFR + 2); // IBFR 2+
  _099(); // 099
}


// ================================================
// 0xb1fe: WORD 'WB200' codep=0x224c wordp=0xb200 params=0 returns=0
// ================================================

void WB200() // WB200
{
  PAD(); // PAD
  Push(0x00c8);
  Push(0x00ff);
  FILL_2(); // FILL_2
  GetDS(); // @DS
  PAD(); // PAD
  Push(Read16(pp_CURSEG)); // CURSEG @
  Push(0);
  Push(0x00c8);
  LCMOVE(); // LCMOVE
}


// ================================================
// 0xb21e: WORD 'XKEYINT' codep=0xb22a wordp=0xb22a
// ================================================
// 0xb22a: push   ax
// 0xb22b: push   ds
// 0xb22c: mov    ax,0040
// 0xb22f: mov    ds,ax
// 0xb231: xor    ax,ax
// 0xb233: in     al,60
// 0xb235: and    ax,0080
// 0xb238: jnz    B242
// 0xb23a: mov    ax,[001C]
// 0xb23e: mov    [001A],ax
// 0xb242: pop    ds
// 0xb243: pop    ax
// 0xb244: jmp    28D0:02BE

// ================================================
// 0xb249: WORD 'INSTALL' codep=0x224c wordp=0xb255 params=0 returns=0
// ================================================

void INSTALL() // INSTALL
{
  Push(Read16(pp__bo_KEYINT)==0?1:0); // [KEYINT @ 0=
  if (Pop() == 0) return;
  Push(9);
  _ro__ex_OLD_rc_(); // (!OLD)
  Push(Read16(pp_W6517)); // W6517 @
  _2_ex__2(); // 2!_2
  Push(pp__bo_KEYINT); // [KEYINT
  ON_2(); // ON_2
  GetDS(); // @DS
  Push(0xb22a); // 'XKEYINT'
  Push(9);
  _ro__ex_SET_rc_(); // (!SET)
}


// ================================================
// 0xb279: WORD 'WB27B' codep=0x224c wordp=0xb27b params=0 returns=0
// ================================================
// orphan

void WB27B() // WB27B
{
  Push(0x01b8);
  _2_at_(); // 2@
  Push(0x001b);
  _ro__ex_SET_rc_(); // (!SET)
}


// ================================================
// 0xb289: WORD 'WB28B' codep=0x224c wordp=0xb28b
// ================================================
// orphan

void WB28B() // WB28B
{
  SETSYSK(); // SETSYSK
  INSTALL(); // INSTALL
  WB066(); // WB066
  WB27B(); // WB27B
}


// ================================================
// 0xb295: WORD 'RESTORE' codep=0x224c wordp=0xb2a1 params=0 returns=0
// ================================================

void RESTORE() // RESTORE
{
  Push(Read16(pp__bo_KEYINT)); // [KEYINT @
  if (Pop() == 0) return;
  Push(Read16(pp_W6517)); // W6517 @
  _2_at_(); // 2@
  Push(9);
  _ro__ex_SET_rc_(); // (!SET)
  Push(pp__bo_KEYINT); // [KEYINT
  _099(); // 099
}


// ================================================
// 0xb2b9: WORD 'WB2BB' codep=0x224c wordp=0xb2bb params=0 returns=0
// ================================================
// orphan

void WB2BB() // WB2BB
{
  RESTORE(); // RESTORE
  WB080(); // WB080
}


// ================================================
// 0xb2c1: WORD 'WB2C3' codep=0x224c wordp=0xb2c3
// ================================================

void WB2C3() // WB2C3
{
  SETSYSK(); // SETSYSK
  Push(Read16(pp_SYSK) * 0x0040 - 1); // SYSK @ 0x0040 * 1-
  Push(pp_LFSEG); // LFSEG
  Store_2(); // !_2
  AINSTALLS(); // AINSTALLS
  Push(pp_REMSEG); // REMSEG
  Push(0x0600);
  BINSTALL(); // BINSTALL
  W8DA5(); // W8DA5
  W8597(); // W8597
}


// ================================================
// 0xb2e5: WORD 'WB2E7' codep=0x224c wordp=0xb2e7
// ================================================

void WB2E7() // WB2E7
{
  GetMODE(); // @MODE
  SETDBUF(); // SETDBUF
  BINSTALLS(); // BINSTALLS
  SETDBUF(); // SETDBUF
}


// ================================================
// 0xb2f1: WORD 'LOAD-DI' codep=0x224c wordp=0xb2fd
// ================================================

void LOAD_dash_DI() // LOAD-DI
{
  NOP(); // NOP
  Push(Read16(pp_DICT)); // DICT @
  Push(6);
  _2DUP(); // 2DUP
  FILE_st_(); // FILE<
  Func6("FILE-#R");
  Push(Read16(Pop()) * 0x0010); //  @ 0x0010 *
  Push(0);
  SWAP(); // SWAP
  _gt_XOR(); // >XOR
}


// ================================================
// 0xb31d: WORD 'LDSILS' codep=0x224c wordp=0xb328
// ================================================

void LDSILS() // LDSILS
{
  Push(Read16(pp_SIL_h_)); // SIL^ @
  Push(0x007d);
  FILE_st_(); // FILE<
}


// ================================================
// 0xb334: WORD '!PAL' codep=0x224c wordp=0xb33d
// ================================================

void StorePAL() // !PAL
{
  IsVGA(); // ?VGA
  if (Pop() == 0) return;
  Push(Read16(pp_PAL_h_)); // PAL^ @
  Push(0x0036);
  FILE_st_(); // FILE<
}


// ================================================
// 0xb34f: WORD 'CONFIGU' codep=0x224c wordp=0xb35b
// ================================================

void CONFIGU() // CONFIGU
{
  SETMAXD(); // SETMAXD
  WB2E7(); // WB2E7
  Push(Read16(pp_LFSEG)); // LFSEG @
  GetDS(); // @DS
  Push(Pop() + 0x1000); //  0x1000 +
  _dash_(); // -
  Push(Read16(regsp)); // DUP
  _0_st_(); // 0<
  if (Pop() != 0)
  {
    Pop(); // DROP
    Push(0);
  } else
  {
    Push(0);
    Push(0x0042);
    U_slash_MOD(); // U/MOD
    SWAP(); // SWAP
    Pop(); // DROP
  }
  Push(Read16(regsp)); // DUP
  Push(pp__bo__n_CACHE); // [#CACHE
  Store_2(); // !_2
  Push(pp__n_CACHE); // #CACHE
  Store_2(); // !_2
  AUTO_dash_CACHE(); // AUTO-CACHE
  Push(Read16(pp_BUFFER_dash_BEGIN)); // BUFFER-BEGIN @
  SEG_gt_ADDR(); // SEG>ADDR
  Push(Read16(regsp)); // DUP
  Push(pp_W5625); // W5625
  Store_2(); // !_2
  Push(Pop() + 0x0410); //  0x0410 +
  Push(pp_W5629); // W5629
  Store_2(); // !_2
  MOUNTA(); // MOUNTA
  LOAD_dash_DI(); // LOAD-DI
  LDSILS(); // LDSILS
  StorePAL(); // !PAL
  W8D05(); // W8D05
  Push(0);
  LDPAL(); // LDPAL
  Push(pp__i_SETUP_plus_); // 'SETUP+
  GetEXECUTE(); // @EXECUTE
  NODRIVES(); // NODRIVES
  ICINIT(); // ICINIT
  WB200(); // WB200
  SETDBUF(); // SETDBUF
}


// ================================================
// 0xb3c7: WORD 'STARTER' codep=0x224c wordp=0xb3d3
// ================================================

void STARTER() // STARTER
{
  WB2C3(); // WB2C3
  DOSPARM(); // DOSPARM
  CONFIGU(); // CONFIGU
  NOP(); // NOP
}


// ================================================
// 0xb3dd: WORD 'DOS' codep=0x224c wordp=0xb3e5 params=0 returns=0
// ================================================

void DOS() // DOS
{
}


// ================================================
// 0xb3e7: WORD '+EDIT' codep=0x1ab5 wordp=0xb3f1
// ================================================
// 0xb3f1: db 0xff 0xb3 0x1c 0x08 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '                    '

// ================================================
// 0xb405: WORD 'EDIT-OV' codep=0x83dd wordp=0xb411
// ================================================
// Overlay EDIT_dash_OV = 0x0db5

// ================================================
// 0xb413: WORD 'EDIT' codep=0x224c wordp=0xb41c
// ================================================

void EDIT() // EDIT
{
  LoadOverlay(EDIT_dash_OV); // EDIT-OV
  DR1(); // DR1
  EDIT(); // Overlay EDIT-OV
}


// ================================================
// 0xb424: WORD 'USE"' codep=0x224c wordp=0xb42d
// ================================================

void USE_qm_() // USE"
{
  USING(); // USING
  DR1(); // DR1
}


// ================================================
// 0xb433: WORD 'SB' codep=0x2214 wordp=0xb43a
// ================================================
// 0xb43a: dw 0x0168

// ================================================
// 0xb43c: WORD 'WB43E' codep=0xb43e wordp=0xb43e
// ================================================
// 0xb43e: mov    ax,ds
// 0xb440: mov    es,ax
// 0xb442: lodsw
// 0xb443: mov    bx,ax
// 0xb445: jmp    word ptr [bx]

// ================================================
// 0xb447: WORD 'WB449' codep=0x224c wordp=0xb449 params=1 returns=1
// ================================================

void WB449() // WB449
{
  BLOCK_2(); // BLOCK_2
  SWAP(); // SWAP
  Push(Pop() * 0x0040); //  0x0040 *
  Push(Pop() + Pop()); // +
}


// ================================================
// 0xb457: WORD 'WB459' codep=0x224c wordp=0xb459
// ================================================

void WB459() // WB459
{
  PAD(); // PAD
  Push(0x0041);
  Push(Read16(cc_BL)); // BL
  FILL_2(); // FILL_2
  Exec("WORD"); // call of word 0x1f06 '(WORD)'
  PAD(); // PAD
  OVER(); // OVER
  Push((Read16(Pop())&0xFF) + 1); //  C@ 1+
  CMOVE_2(); // CMOVE_2
}


// ================================================
// 0xb471: WORD 'WB473' codep=0x224c wordp=0xb473 params=0 returns=1
// ================================================

void WB473() // WB473
{
  Push(Read16(user_SCR)); // SCR @
  WB449(); // WB449
}


// ================================================
// 0xb47b: WORD 'WB47D' codep=0x224c wordp=0xb47d params=0 returns=2
// ================================================

void WB47D() // WB47D
{
  Push(Read16(user_R_n_)); // R# @
  Push(0x0040);
  _slash_MOD(); // /MOD
}


// ================================================
// 0xb489: WORD 'WB48B' codep=0x224c wordp=0xb48b params=0 returns=3
// ================================================

void WB48B() // WB48B
{
  WB47D(); // WB47D
  WB473(); // WB473
  SWAP(); // SWAP
}


// ================================================
// 0xb493: WORD 'WB495' codep=0x224c wordp=0xb495 params=0 returns=3
// ================================================

void WB495() // WB495
{
  unsigned short int a;
  WB48B(); // WB48B
  Push(Read16(regsp)); // DUP
  a = Pop(); // >R
  Push(Pop() + Pop()); // +
  Push(0x0040 - a); // 0x0040 R> -
}


// ================================================
// 0xb4a7: WORD 'WB4A9' codep=0x224c wordp=0xb4a9 params=1 returns=2
// ================================================

void WB4A9() // WB4A9
{
  Push(user_R_n_); // R#
  _plus__ex__2(); // +!_2
  Exec("CR"); // call of word 0x26ee '(CR)'
  SPACE(); // SPACE
  WB48B(); // WB48B
  Exec("TYPE"); // call of word 0x2690 '(TYPE)'
  Push(0x005f);
  Exec("EMIT"); // call of word 0x2731 '(EMIT)'
  WB495(); // WB495
  Exec("TYPE"); // call of word 0x2690 '(TYPE)'
}


// ================================================
// 0xb4c1: WORD 'WB4C3' codep=0x224c wordp=0xb4c3
// ================================================

void WB4C3() // WB4C3
{
  WB495(); // WB495
  PAD(); // PAD
  COUNT(); // COUNT
  Push(1);
  MAX(); // MAX
  WB43E(); // WB43E
  MATCH(); // MATCH
  Push(user_R_n_); // R#
  _plus__ex__2(); // +!_2
}


// ================================================
// 0xb4d7: WORD 'WB4D9' codep=0x224c wordp=0xb4d9
// ================================================

void WB4D9() // WB4D9
{
  unsigned short int i, imax;
  Push(0x007c);
  WB459(); // WB459
  Push(Pop() + 1); //  1+
  SWAP(); // SWAP

  i = Pop();
  imax = Pop();
  do // (DO)
  {
    Push(i); // I
    Push(user_SCR); // SCR
    Store_2(); // !_2
    PRINT(".", 1); // (.")
    Push(0);
    Push(user_R_n_); // R#
    Store_2(); // !_2
    do
    {
      WB4C3(); // WB4C3
      if (Pop() != 0)
      {
        Exec("CR"); // call of word 0x26ee '(CR)'
        Exec("CR"); // call of word 0x26ee '(CR)'
        Push(Read16(user_SCR)); // SCR @
        Push(3);
        DrawR(); // .R
        SPACE(); // SPACE
        WB47D(); // WB47D
        Push(2);
        DrawR(); // .R
        Pop(); // DROP
        Push(1);
        WB4A9(); // WB4A9
      }
      Push(0x03ff);
      Push(Read16(user_R_n_)); // R# @
      _st_(); // <
    } while(Pop() == 0);
    Exec("IsTERMINAL"); // call of word 0x25bc '(?TERMINAL)'
    if (Pop() != 0)
    {
      imax = i; // LEAVE
    }
    i++;
  } while(i<imax); // (LOOP)

}


// ================================================
// 0xb531: WORD 'WB533' codep=0x224c wordp=0xb533 params=0 returns=2
// ================================================

void WB533() // WB533
{
  Push(0 + Read16(cc_SB)); // 0 SB +
  FILESIZ(); // FILESIZ
  _2_at_(); // 2@
  SWAP(); // SWAP
  Push(0x0400);
  M_slash_(); // M/
  SWAP(); // SWAP
  Pop(); // DROP
  Push((Pop() - 1) + Read16(cc_SB)); //  1- SB +
}


// ================================================
// 0xb551: WORD 'WS' codep=0x224c wordp=0xb558
// ================================================

void WS() // WS
{
  WB533(); // WB533
  WB4D9(); // WB4D9
}


// ================================================
// 0xb55e: WORD 'WL' codep=0x224c wordp=0xb565
// ================================================

void WL() // WL
{
  WB533(); // WB533
  THRU_2(); // THRU_2
}


// ================================================
// 0xb56b: WORD 'WB56D' codep=0x224c wordp=0xb56d
// ================================================

void WB56D() // WB56D
{
  unsigned short int i, imax;

  i = Pop() - 2; //  2-
  imax = Pop();
  do // (DO)
  {
    Push(Read16(i)); // I @
    Push(0);
    D_dot_(); // D.
    Push(-2);
    int step = Pop();
    i += step;
    if (((step>=0) && (i>=imax)) || ((step<0) && (i<=imax))) break;
  } while(1); // (+LOOP)

}


// ================================================
// 0xb581: WORD '.S' codep=0x224c wordp=0xb588 params=0 returns=0
// ================================================

void DrawS() // .S
{
  Exec("CR"); // call of word 0x26ee '(CR)'
  DEPTH(); // DEPTH
  if (Pop() == 0) return;
  SP_at_(); // SP@
  S0(); // S0
  WB56D(); // WB56D
}


// ================================================
// 0xb598: WORD 'MUSIC' codep=0x1ab5 wordp=0xb5a2
// ================================================
// 0xb5a2: db 0xb0 0xb5 0xf3 0xb3 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '                    '

// ================================================
// 0xb5b6: WORD 'MUSIC-OV' codep=0x83dd wordp=0xb5c3
// ================================================
// Overlay MUSIC_dash_OV = 0x3bc4

// ================================================
// 0xb5c5: WORD 'INTROS' codep=0x224c wordp=0xb5d0
// ================================================

void INTROS() // INTROS
{
  LoadOverlay(MUSIC_dash_OV); // MUSIC-OV
  INTROS(); // Overlay MUSIC
}


// ================================================
// 0xb5d6: WORD 'HIMUS' codep=0x224c wordp=0xb5e0
// ================================================

void HIMUS() // HIMUS
{
  LoadOverlay(MUSIC_dash_OV); // MUSIC-OV
  HIMUS(); // Overlay MUSIC
}


// ================================================
// 0xb5e6: WORD 'TALLOC' codep=0x224c wordp=0xb5f1
// ================================================

void TALLOC() // TALLOC
{
  LoadOverlay(MUSIC_dash_OV); // MUSIC-OV
  TANDY_dash_ALLOC(); // Overlay MUSIC
}


// ================================================
// 0xb5f7: WORD 'SETABLT' codep=0x224c wordp=0xb603
// ================================================

void SETABLT() // SETABLT
{
  LoadOverlay(MUSIC_dash_OV); // MUSIC-OV
  SETABLT(); // Overlay MUSIC
}


// ================================================
// 0xb609: WORD '*OP' codep=0x3b74 wordp=0xb611
// ================================================
// 0xb611: dw 0x0002 0x2b71

// ================================================
// 0xb615: WORD '*COMM' codep=0x3b74 wordp=0xb61f
// ================================================
// 0xb61f: dw 0x0001 0xacba

// ================================================
// 0xb623: WORD '*EOL' codep=0x3b74 wordp=0xb62c
// ================================================
// 0xb62c: dw 0x0002 0x522f

// ================================================
// 0xb630: WORD 'OTABL' codep=0x1d29 wordp=0xb63a
// ================================================
// 0xb63a: db 0x1c 0x71 0x01 0xa7 0x48 0x01 0x75 0xd2 0x00 0x93 0xd2 0x00 0xfd 0xda 0x00 0x96 0x2e 0x01 0x3a 0x71 0x01 0x84 0xfa 0x00 0xc7 0xae 0x00 0x03 0xaf 0x00 0x29 0x20 0x01 0x20 0x20 0x20 0xce 0x9c 0x00 0x73 0x97 0x00 0xa8 0xab 0x01 0x6e 0xae 0x00 0xe5 0xae 0x00 0x8a 0xab 0x01 ' q  H u         . :q          )        s     n        '

// ================================================
// 0xb670: WORD '+ORG' codep=0x4b3b wordp=0xb679
// ================================================

void _plus_ORG() // +ORG
{
  switch(Pop()) // +ORG
  {
  case 18:
    Push(0);
    break;
  case 19:
    Push(1);
    break;
  case 21:
    Push(2);
    break;
  case 22:
    Push(3);
    break;
  case 23:
    Push(4);
    break;
  case 24:
    Push(5);
    break;
  default:
    Push(-1);
    break;

  }
}

// ================================================
// 0xb695: WORD 'ORGIA' codep=0x224c wordp=0xb69f
// ================================================

void ORGIA() // ORGIA
{
  Push(Read16(regsp)); // DUP
  Push(0x000c);
  MIN(); // MIN
  SWAP(); // SWAP
  _plus_ORG(); // +ORG case
  Push(Pop() + Pop()); // +
  Push(Pop() * 3 + pp_OTABL); //  3 * OTABL +
}


// ================================================
// 0xb6b7: WORD 'MISC-VOC' codep=0x1ab5 wordp=0xb6c1
// ================================================
// 0xb6c1: db 0xcf 0xb6 0xa4 0xb5 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '                    '

// ================================================
// 0xb6d5: WORD 'MISC-OV' codep=0x83dd wordp=0xb6df
// ================================================
// Overlay MISC_dash_OV = 0x729c

// ================================================
// 0xb6e1: WORD '=CARGO' codep=0x224c wordp=0xb6ec
// ================================================

void _eq_CARGO() // =CARGO
{
  LoadOverlay(MISC_dash_OV); // MISC-OV
  _eq_CARG(); // Overlay MISC-OV
}


// ================================================
// 0xb6f2: WORD 'OV#IN$' codep=0x224c wordp=0xb6fd
// ================================================

void OV_n_IN_do_() // OV#IN$
{
  LoadOverlay(MISC_dash_OV); // MISC-OV
  _ro__n_IN_do_(); // Overlay MISC-OV
}


// ================================================
// 0xb703: WORD 'OVQUIT' codep=0x224c wordp=0xb70e
// ================================================

void OVQUIT() // OVQUIT
{
  LoadOverlay(MISC_dash_OV); // MISC-OV
  _ro_QUIT(); // Overlay MISC-OV
}


// ================================================
// 0xb714: WORD 'OV.0$$' codep=0x224c wordp=0xb71f
// ================================================

void OV_dot_0_do__do_() // OV.0$$
{
  LoadOverlay(MISC_dash_OV); // MISC-OV
  _ro__dot_0_do__do_(); // Overlay MISC-OV
}


// ================================================
// 0xb725: WORD 'OV0$$$' codep=0x224c wordp=0xb730
// ================================================

void OV0_do__do__do_() // OV0$$$
{
  LoadOverlay(MISC_dash_OV); // MISC-OV
  _ro_0_do__do__do_(); // Overlay MISC-OV
}


// ================================================
// 0xb736: WORD 'OVTRIM_2' codep=0x224c wordp=0xb741
// ================================================

void OVTRIM_2() // OVTRIM_2
{
  LoadOverlay(MISC_dash_OV); // MISC-OV
  _ro_TRIM(); // Overlay MISC-OV
}


// ================================================
// 0xb747: WORD 'OVTRIM_1' codep=0x224c wordp=0xb752
// ================================================

void OVTRIM_1() // OVTRIM_1
{
  unsigned short int i, imax;
  LoadOverlay(MISC_dash_OV); // MISC-OV

  i = 0;
  imax = Pop();
  do // (DO)
  {
    _ro_TRIM(); // Overlay MISC-OV
    i++;
  } while(i<imax); // (LOOP)

}


// ================================================
// 0xb760: WORD 'OV?ART' codep=0x224c wordp=0xb76b
// ================================================

void OV_ask_ART() // OV?ART
{
  LoadOverlay(MISC_dash_OV); // MISC-OV
  _ro__ask_ART(); // Overlay MISC-OV
}


// ================================================
// 0xb771: WORD 'OV?.EQ' codep=0x224c wordp=0xb77c
// ================================================

void OV_ask__dot_EQ() // OV?.EQ
{
  LoadOverlay(MISC_dash_OV); // MISC-OV
  _ro__ask__dot_EQ(); // Overlay MISC-OV
}


// ================================================
// 0xb782: WORD 'TV?ART' codep=0x224c wordp=0xb78d
// ================================================

void TV_ask_ART() // TV?ART
{
  LoadOverlay(MISC_dash_OV); // MISC-OV
  TV_ask_AR(); // Overlay MISC-OV
}


// ================================================
// 0xb793: WORD 'RSTE' codep=0x224c wordp=0xb79c
// ================================================

void RSTE() // RSTE
{
  LoadOverlay(MISC_dash_OV); // MISC-OV
  RSTE(); // Overlay MISC-OV
}


// ================================================
// 0xb7a2: WORD 'SCON-V' codep=0x1ab5 wordp=0xb7ad
// ================================================
// 0xb7ad: db 0xbb 0xb7 0xc3 0xb6 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '                    '

// ================================================
// 0xb7c1: WORD 'SCON-O' codep=0x83dd wordp=0xb7cc
// ================================================
// Overlay SCON_dash_O = 0x95b0

// ================================================
// 0xb7ce: WORD 'TOW' codep=0x224c wordp=0xb7d6
// ================================================

void TOW() // TOW
{
  LoadOverlay(SCON_dash_O); // SCON-O
  TOW_dash_US(); // Overlay SHIPCON
}


// ================================================
// 0xb7dc: WORD '.NOFUN' codep=0x224c wordp=0xb7e7
// ================================================

void DrawNOFUN() // .NOFUN
{
  LoadOverlay(SCON_dash_O); // SCON-O
  DrawNOFUN(); // Overlay SHIPCON
}


// ================================================
// 0xb7ed: WORD 'FX-VOC' codep=0x1ab5 wordp=0xb7f8
// ================================================
// 0xb7f8: db 0x06 0xb8 0xaf 0xb7 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '                    '

// ================================================
// 0xb80c: WORD 'FX-OV' codep=0x83dd wordp=0xb816
// ================================================
// Overlay FX_dash_OV = 0x6d04

// ================================================
// 0xb818: WORD 'PSYCH' codep=0x224c wordp=0xb822
// ================================================

void PSYCH() // PSYCH
{
  LoadOverlay(FX_dash_OV); // FX-OV
  PSYCH(); // Overlay FX
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xb82a: WORD '.TMAP' codep=0x224c wordp=0xb834
// ================================================

void DrawTMAP() // .TMAP
{
  LoadOverlay(FX_dash_OV); // FX-OV
  DrawTMAP(); // Overlay FX
}


// ================================================
// 0xb83a: WORD 'IT-VOC' codep=0x1ab5 wordp=0xb845
// ================================================
// 0xb845: db 0x53 0xb8 0xfa 0xb7 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 'S                   '

// ================================================
// 0xb859: WORD 'IT-OV' codep=0x83dd wordp=0xb863
// ================================================
// Overlay IT_dash_OV = 0x9155

// ================================================
// 0xb865: WORD 'WB867' codep=0x224c wordp=0xb867
// ================================================

void WB867() // WB867
{
  LoadOverlay(IT_dash_OV); // IT-OV
  DATE_do__gt_AD(); // Overlay IT-OV
}


// ================================================
// 0xb86d: WORD 'WB86F' codep=0x224c wordp=0xb86f
// ================================================

void WB86F() // WB86F
{
  LoadOverlay(IT_dash_OV); // IT-OV
  BOX_gt_LIST(); // Overlay IT-OV
}


// ================================================
// 0xb875: WORD 'WB877' codep=0x224c wordp=0xb877
// ================================================

void WB877() // WB877
{
  LoadOverlay(IT_dash_OV); // IT-OV
  GET_dash_BOXE(); // Overlay IT-OV
}


// ================================================
// 0xb87d: WORD 'WB87F' codep=0x224c wordp=0xb87f
// ================================================

void WB87F() // WB87F
{
  LoadOverlay(IT_dash_OV); // IT-OV
  MAKE_dash_SCR(); // Overlay IT-OV
}


// ================================================
// 0xb885: WORD 'WB887' codep=0x224c wordp=0xb887
// ================================================

void WB887() // WB887
{
  LoadOverlay(IT_dash_OV); // IT-OV
  DELETE_dash_S(); // Overlay IT-OV
}


// ================================================
// 0xb88d: WORD 'WB88F' codep=0x224c wordp=0xb88f
// ================================================

void WB88F() // WB88F
{
  LoadOverlay(IT_dash_OV); // IT-OV
  BOX_gt_TOCS(); // Overlay IT-OV
}


// ================================================
// 0xb895: WORD 'WB897' codep=0x224c wordp=0xb897
// ================================================

void WB897() // WB897
{
  LoadOverlay(IT_dash_OV); // IT-OV
  _gt_BOX(); // Overlay IT-OV
}


// ================================================
// 0xb89d: WORD 'WB89F' codep=0x224c wordp=0xb89f
// ================================================

void WB89F() // WB89F
{
  LoadOverlay(IT_dash_OV); // IT-OV
  BOX_gt_(); // Overlay IT-OV
}


// ================================================
// 0xb8a5: WORD 'WB8A7' codep=0x224c wordp=0xb8a7
// ================================================

void WB8A7() // WB8A7
{
  LoadOverlay(IT_dash_OV); // IT-OV
  _ro_BOX_gt__rc_(); // Overlay IT-OV
}


// ================================================
// 0xb8ad: WORD 'WB8AF' codep=0x224c wordp=0xb8af
// ================================================

void WB8AF() // WB8AF
{
  LoadOverlay(IT_dash_OV); // IT-OV
  CLASS_gt_BO(); // Overlay IT-OV
}


// ================================================
// 0xb8b5: WORD 'WB8B7' codep=0x224c wordp=0xb8b7
// ================================================

void WB8B7() // WB8B7
{
  LoadOverlay(IT_dash_OV); // IT-OV
  ITEM_gt_PAD(); // Overlay IT-OV
}


// ================================================
// 0xb8bd: WORD 'DATE$>A' codep=0x224c wordp=0xb8c9 params=0 returns=0
// ================================================

void DATE_do__gt_A() // DATE$>A
{
  Push(0xb867); // 'WB867'
  MODULE(); // MODULE
}


// ================================================
// 0xb8d1: WORD 'BOX>LIS' codep=0x224c wordp=0xb8dd params=0 returns=0
// ================================================

void BOX_gt_LIS() // BOX>LIS
{
  Push(0xb86f); // 'WB86F'
  MODULE(); // MODULE
}


// ================================================
// 0xb8e5: WORD 'GET-BOX' codep=0x224c wordp=0xb8f1 params=0 returns=0
// ================================================

void GET_dash_BOX() // GET-BOX
{
  Push(0xb877); // 'WB877'
  MODULE(); // MODULE
}


// ================================================
// 0xb8f9: WORD 'MAKE-SC' codep=0x224c wordp=0xb905 params=0 returns=0
// ================================================

void MAKE_dash_SC() // MAKE-SC
{
  Push(0xb87f); // 'WB87F'
  MODULE(); // MODULE
}


// ================================================
// 0xb90d: WORD 'DELETE-' codep=0x224c wordp=0xb919 params=0 returns=0
// ================================================

void DELETE_dash_() // DELETE-
{
  Push(0xb887); // 'WB887'
  MODULE(); // MODULE
}


// ================================================
// 0xb921: WORD 'BOX>TOC' codep=0x224c wordp=0xb92d params=0 returns=0
// ================================================

void BOX_gt_TOC() // BOX>TOC
{
  Push(0xb88f); // 'WB88F'
  MODULE(); // MODULE
}


// ================================================
// 0xb935: WORD '>BOX' codep=0x224c wordp=0xb93e params=0 returns=0
// ================================================

void _gt_BOX() // >BOX
{
  Push(0xb897); // 'WB897'
  MODULE(); // MODULE
}


// ================================================
// 0xb946: WORD 'BOX>' codep=0x224c wordp=0xb94f params=0 returns=0
// ================================================

void BOX_gt_() // BOX>
{
  Push(0xb89f); // 'WB89F'
  MODULE(); // MODULE
}


// ================================================
// 0xb957: WORD '(BOX>)' codep=0x224c wordp=0xb962 params=0 returns=0
// ================================================

void _ro_BOX_gt__rc_() // (BOX>)
{
  Push(0xb8a7); // 'WB8A7'
  MODULE(); // MODULE
}


// ================================================
// 0xb96a: WORD 'CLASS>B' codep=0x224c wordp=0xb976 params=0 returns=0
// ================================================

void CLASS_gt_B() // CLASS>B
{
  Push(0xb8af); // 'WB8AF'
  MODULE(); // MODULE
}


// ================================================
// 0xb97e: WORD 'ITEM>PA' codep=0x224c wordp=0xb98a params=0 returns=0
// ================================================

void ITEM_gt_PA() // ITEM>PA
{
  Push(0xb8b7); // 'WB8B7'
  MODULE(); // MODULE
}


// ================================================
// 0xb992: WORD 'BLT-VOC' codep=0x1ab5 wordp=0xb99e
// ================================================
// 0xb99e: db 0xac 0xb9 0x47 0xb8 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '  G                 '

// ================================================
// 0xb9b2: WORD 'BLT-' codep=0x83dd wordp=0xb9bb
// ================================================
// Overlay BLT_dash_ = 0x7244

// ================================================
// 0xb9bd: WORD '@.HYBR' codep=0x224c wordp=0xb9c8
// ================================================

void Get_dot_HYBR() // @.HYBR
{
  LoadOverlay(BLT_dash_); // BLT-
  Get_dot_HY(); // Overlay BLT-OV
}


// ================================================
// 0xb9ce: WORD '.TPIC' codep=0x224c wordp=0xb9d8
// ================================================

void DrawTPIC() // .TPIC
{
  LoadOverlay(BLT_dash_); // BLT-
  DrawTPI(); // Overlay BLT-OV
}


// ================================================
// 0xb9de: WORD '.APIC' codep=0x224c wordp=0xb9e8
// ================================================

void DrawAPIC() // .APIC
{
  LoadOverlay(BLT_dash_); // BLT-
  DrawAPI(); // Overlay BLT-OV
}


// ================================================
// 0xb9ee: WORD 'SETBLT' codep=0x224c wordp=0xb9f9
// ================================================

void SETBLT() // SETBLT
{
  LoadOverlay(BLT_dash_); // BLT-
  SETB(); // Overlay BLT-OV
}


// ================================================
// 0xb9ff: WORD 'LDAPIC' codep=0x224c wordp=0xba0a
// ================================================

void LDAPIC() // LDAPIC
{
  LoadOverlay(BLT_dash_); // BLT-
  LDAP(); // Overlay BLT-OV
}


// ================================================
// 0xba10: WORD 'LDSYS' codep=0x224c wordp=0xba1a
// ================================================

void LDSYS() // LDSYS
{
  LoadOverlay(BLT_dash_); // BLT-
  LDSY(); // Overlay BLT-OV
}


// ================================================
// 0xba20: WORD 'VICONP' codep=0x1ab5 wordp=0xba2b
// ================================================
// 0xba2b: db 0x39 0xba 0xa0 0xb9 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '9                   '

// ================================================
// 0xba3f: WORD 'ICONP' codep=0x83dd wordp=0xba49
// ================================================
// Overlay ICONP = 0x70c6

// ================================================
// 0xba4b: WORD 'ICON-' codep=0x224c wordp=0xba55
// ================================================

void ICON_dash_() // ICON-
{
  LoadOverlay(ICONP); // ICONP
  ICON_dash_PARM(); // Overlay ICONP-OV
}


// ================================================
// 0xba5b: WORD '+ICON_1' codep=0x224c wordp=0xba65
// ================================================

void _plus_ICON_1() // +ICON_1
{
  LoadOverlay(ICONP); // ICONP
  _plus_ICONBOX(); // Overlay ICONP-OV
}


// ================================================
// 0xba6b: WORD 'FRACT' codep=0x1ab5 wordp=0xba75
// ================================================
// 0xba75: db 0x83 0xba 0x2d 0xba 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '  -                 '

// ================================================
// 0xba89: WORD 'FRACT-OV' codep=0x83dd wordp=0xba96
// ================================================
// Overlay FRACT_dash_OV = 0x6fa4

// ================================================
// 0xba98: WORD 'MERCA' codep=0x224c wordp=0xbaa2
// ================================================

void MERCA() // MERCA
{
  LoadOverlay(FRACT_dash_OV); // FRACT-OV
  MERCATOR_dash_GEN(); // Overlay FRACT-OV
}


// ================================================
// 0xbaa8: WORD 'NEWCO' codep=0x224c wordp=0xbab2
// ================================================

void NEWCO() // NEWCO
{
  LoadOverlay(FRACT_dash_OV); // FRACT-OV
  NEWCONTOUR(); // Overlay FRACT-OV
}


// ================================================
// 0xbab8: WORD 'MAKE-' codep=0x224c wordp=0xbac2
// ================================================

void MAKE_dash_() // MAKE-
{
  LoadOverlay(FRACT_dash_OV); // FRACT-OV
  MAKE_dash_GLOBE(); // Overlay FRACT-OV
}


// ================================================
// 0xbac8: WORD 'SCRLC' codep=0x224c wordp=0xbad2
// ================================================

void SCRLC() // SCRLC
{
  LoadOverlay(FRACT_dash_OV); // FRACT-OV
  SCRLCON(); // Overlay FRACT-OV
}


// ================================================
// 0xbad8: WORD 'BTADDR' codep=0x1d29 wordp=0xbae3
// ================================================
// 0xbae3: db 0xfc 0xbb '  '

// ================================================
// 0xbae5: WORD 'HUFF_BT_POINTER' codep=0x1d29 wordp=0xbae7
// ================================================
// 0xbae7: db 0x3a 0x20 ': '

// ================================================
// 0xbae9: WORD 'HUFF_ACTIVE_POINTER_BYTE' codep=0x1d29 wordp=0xbaeb
// ================================================
// 0xbaeb: db 0x3a 0x20 ': '

// ================================================
// 0xbaed: WORD 'WBAEF' codep=0x1d29 wordp=0xbaef
// ================================================
// 0xbaef: db 0x3a 0x20 ': '

// ================================================
// 0xbaf1: WORD 'HUFF_ACTIVE_POINTER_BIT' codep=0x1d29 wordp=0xbaf3
// ================================================
// 0xbaf3: db 0x3a 0x20 ': '

// ================================================
// 0xbaf5: WORD 'HUFF_PHRASE_ADDCHAR' codep=0x2214 wordp=0xbaf7
// ================================================
// 0xbaf7: dw 0xbbda

// ================================================
// 0xbaf9: WORD 'HUFF_PHRASE_INIT' codep=0x2214 wordp=0xbafb
// ================================================
// 0xbafb: dw 0xbbd2

// ================================================
// 0xbafd: WORD 'HUFF_PHRASE_FINISH' codep=0x2214 wordp=0xbaff
// ================================================
// 0xbaff: dw 0xbbf0

// ================================================
// 0xbb01: WORD 'HUFF_MOVE_ACTIVE_POINTER' codep=0x224c wordp=0xbb03 params=0 returns=0
// ================================================

void HUFF_MOVE_ACTIVE_POINTER() // HUFF_MOVE_ACTIVE_POINTER
{
  Push(Read16(pp_HUFF_ACTIVE_POINTER_BIT)); // HUFF_ACTIVE_POINTER_BIT @
  Push(Read16(regsp)); // DUP
  Push(Pop()==0x0080?1:0); //  0x0080 =
  Push(pp_HUFF_ACTIVE_POINTER_BYTE); // HUFF_ACTIVE_POINTER_BYTE
  _plus__ex__2(); // +!_2
  Push(Pop() >> 1); //  2/
  Push(Read16(regsp)); // DUP
  Push((Pop()==0?1:0) * 0x0080); //  0= 0x0080 *
  Push(Pop() + Pop()); // +
  Push(pp_HUFF_ACTIVE_POINTER_BIT); // HUFF_ACTIVE_POINTER_BIT
  Store_2(); // !_2
}


// ================================================
// 0xbb27: WORD 'WBB29' codep=0x224c wordp=0xbb29 params=1 returns=2
// ================================================

void WBB29() // WBB29
{
  Push(Pop() & (Read16(pp_HUFF_ACTIVE_POINTER_BIT)&0xFF)); //  HUFF_ACTIVE_POINTER_BIT C@ AND
  _0_gt_(); // 0>
  Push(Read16((Pop() + Read16(pp_BTADDR)) + Read16(pp_HUFF_BT_POINTER))&0xFF); //  BTADDR @ + HUFF_BT_POINTER @ + C@
  Push(Read16(regsp)); // DUP
  Push(Pop() & 0x0080); //  0x0080 AND
  if (Pop() != 0)
  {
    Push((Pop() & 0x007f) * 2); //  0x007f AND 2*
    Push(pp_HUFF_BT_POINTER); // HUFF_BT_POINTER
    _plus__ex__2(); // +!_2
  } else
  {
    Push(1);
    Push(pp_WBAEF); // WBAEF
    _plus__ex__2(); // +!_2
    Push(Read16(cc_HUFF_PHRASE_ADDCHAR)); // HUFF_PHRASE_ADDCHAR
    EXECUTE(); // EXECUTE
    Push(pp_HUFF_BT_POINTER); // HUFF_BT_POINTER
    _099(); // 099
  }
  HUFF_MOVE_ACTIVE_POINTER(); // HUFF_MOVE_ACTIVE_POINTER
  Push(Read16(pp_HUFF_ACTIVE_POINTER_BIT)==0x0080?1:0); // HUFF_ACTIVE_POINTER_BIT @ 0x0080 =
}


// ================================================
// 0xbb77: WORD 'HUFF>' codep=0x224c wordp=0xbb81 params=2 returns=0
// ================================================

void HUFF_gt_() // HUFF>
{
  Push(pp_HUFF_BT_POINTER); // HUFF_BT_POINTER
  _099(); // 099
  Push(pp_WBAEF); // WBAEF
  _099(); // 099
  Push(pp_HUFF_ACTIVE_POINTER_BYTE); // HUFF_ACTIVE_POINTER_BYTE
  _099(); // 099
  Push(0x0080);
  Push(pp_HUFF_ACTIVE_POINTER_BIT); // HUFF_ACTIVE_POINTER_BIT
  Store_2(); // !_2
  Push(Read16(cc_HUFF_PHRASE_INIT)); // HUFF_PHRASE_INIT
  EXECUTE(); // EXECUTE
  while(1)
  {
    Push(Read16(regsp)); // DUP
    Push(Read16(pp_WBAEF)); // WBAEF @
    _gt_(); // >
    if (Pop() == 0) break;

    do
    {
      OVER(); // OVER
      Push(Read16(Pop() + Read16(pp_HUFF_ACTIVE_POINTER_BYTE))&0xFF); //  HUFF_ACTIVE_POINTER_BYTE @ + C@
      WBB29(); // WBB29
    } while(Pop() == 0);
  }
  Pop(); Pop(); // 2DROP
  Push(Read16(cc_HUFF_PHRASE_FINISH)); // HUFF_PHRASE_FINISH
  EXECUTE(); // EXECUTE
}


// ================================================
// 0xbbc1: WORD 'PHRASE$' codep=0x7420 wordp=0xbbcd
// ================================================
IFieldType PHRASE_do_ = {STRINGIDX, 0x0b, 0x00};

// ================================================
// 0xbbd0: WORD 'HUFF_LSCAN_INIT' codep=0x224c wordp=0xbbd2 params=0 returns=0
// ================================================
// orphan

void HUFF_LSCAN_INIT() // HUFF_LSCAN_INIT
{
  Push(pp_LSCAN); // LSCAN
  _099(); // 099
}


// ================================================
// 0xbbd8: WORD 'HUFF_LSCAN_ADDCHAR' codep=0x224c wordp=0xbbda params=1 returns=0
// ================================================
// orphan

void HUFF_LSCAN_ADDCHAR() // HUFF_LSCAN_ADDCHAR
{
  Push(pp_LSCAN); // LSCAN
  Push(Read16(regsp)); // DUP
  Push(Read16(Pop())&0xFF); //  C@
  Push(Pop() + Pop()); // +
  Push(Pop() + 1); //  1+
  C_ex__2(); // C!_2
  Push(1);
  Push(pp_LSCAN); // LSCAN
  _plus__ex__2(); // +!_2
}


// ================================================
// 0xbbee: WORD 'HUFF_LSCAN_FINISH' codep=0x224c wordp=0xbbf0 params=0 returns=0
// ================================================
// orphan

void HUFF_LSCAN_FINISH() // HUFF_LSCAN_FINISH
{
  Push(Read16((0x65e1+PHRASE_do_.offset) + 1)&0xFF); // PHRASE$<IFIELD> 1+ C@
  Push(pp_LSCAN); // LSCAN
  C_ex__2(); // C!_2
}

// 0xbbfc: db 0xa1 0x81 0x8b 0x81 0x5e 0x81 0x81 0x45 0x81 0x52 0x81 0x44 0x81 0x56 0x4b 0x81 0x81 0x30 0x81 0x2d 0x81 0x4a 0x36 0x38 0x81 0x20 0x83 0x81 0x81 0x48 0x59 0x43 0x82 0x81 0x4d 0x57 0x81 0x2e 0x81 0x42 0x84 0x81 0x31 0x81 0x5a 0x81 0x34 0x33 0x86 0x81 0x51 0x81 0x81 0x32 0x81 0x25 0x2b 0x81 0x23 0x28 0x81 0x3f 0x39 0x81 0x37 0x2a 0x86 0x81 0x84 0x81 0x54 0x81 0x81 0x4c 0x50 0x46 0x53 0x4f 0x82 0x81 0x4e 0x41 0x49 0x81 0x81 0x55 0x81 0x47 0x2c 0x81 0x81 0x58 0x27 0x81 0x81 0x21 0x35 0x81 0x26 0x81 0x29 0x2f '    ^  E R D VK  0 - J68     HYC  MW . B  1 Z 43  Q  2 %+ #( ?9 7*    T  LPFSO  NAI  U G,  X'  !5 & )/'

// ================================================
// 0xbc62: WORD 'ENG-VOC' codep=0x1ab5 wordp=0xbc6e
// ================================================
// 0xbc6e: db 0x7c 0xbc 0x77 0xba 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '| w                 '

// ================================================
// 0xbc82: WORD 'ENG-O' codep=0x83dd wordp=0xbc8c
// ================================================
// Overlay ENG_dash_O = 0x7f51

// ================================================
// 0xbc8e: WORD 'OV/DA' codep=0x224c wordp=0xbc98
// ================================================

void OV_slash_DA() // OV/DA
{
  LoadOverlay(ENG_dash_O); // ENG-O
  _ro__slash_DAMAGE_rc_(); // Overlay ENGINEER
}


// ================================================
// 0xbc9e: WORD 'OV/RE' codep=0x224c wordp=0xbca8
// ================================================

void OV_slash_RE() // OV/RE
{
  LoadOverlay(ENG_dash_O); // ENG-O
  _ro__slash_REPAIR_rc_(); // Overlay ENGINEER
}


// ================================================
// 0xbcae: WORD 'OVDBA' codep=0x224c wordp=0xbcb8
// ================================================

void OVDBA() // OVDBA
{
  LoadOverlay(ENG_dash_O); // ENG-O
  DBARS(); // Overlay ENGINEER
}


// ================================================
// 0xbcbe: WORD 'SETTER' codep=0x1ab5 wordp=0xbcc9
// ================================================
// 0xbcc9: db 0xd7 0xbc 0x70 0xbc 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '  p                 '

// ================================================
// 0xbcdd: WORD 'PLSET-OV' codep=0x83dd wordp=0xbcea
// ================================================
// Overlay PLSET_dash_OV = 0x8cdb

// ================================================
// 0xbcec: WORD 'SET-PLAN' codep=0x224c wordp=0xbcf9
// ================================================

void SET_dash_PLAN() // SET-PLAN
{
  LoadOverlay(PLSET_dash_OV); // PLSET-OV
  PLAN_dash_RUL(); // Overlay PLSET-OV
}


// ================================================
// 0xbcff: WORD 'ORBIT_2' codep=0x1ab5 wordp=0xbd09
// ================================================
// 0xbd09: db 0x17 0xbd 0xcb 0xbc 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '                    '

// ================================================
// 0xbd1d: WORD 'ORBIT_1' codep=0x83dd wordp=0xbd27
// ================================================
// Overlay ORBIT_1 = 0x802d

// ================================================
// 0xbd29: WORD 'INIT-' codep=0x224c wordp=0xbd33
// ================================================

void INIT_dash_() // INIT-
{
  LoadOverlay(ORBIT_1); // ORBIT_1
  INIT_dash_ORBIT(); // Overlay ORBIT-OV
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xbd3b: WORD 'SHOWS' codep=0x224c wordp=0xbd45
// ================================================

void SHOWS() // SHOWS
{
  LoadOverlay(ORBIT_1); // ORBIT_1
  SHOWSITE(); // Overlay ORBIT-OV
}


// ================================================
// 0xbd4b: WORD 'DESCE' codep=0x224c wordp=0xbd55
// ================================================

void DESCE() // DESCE
{
  LoadOverlay(ORBIT_1); // ORBIT_1
  DESCEND(); // Overlay ORBIT-OV
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xbd5d: WORD 'ASCEN' codep=0x224c wordp=0xbd67
// ================================================

void ASCEN() // ASCEN
{
  LoadOverlay(ORBIT_1); // ORBIT_1
  ASCEND(); // Overlay ORBIT-OV
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xbd6f: WORD 'ROTAT' codep=0x224c wordp=0xbd79
// ================================================

void ROTAT() // ROTAT
{
  LoadOverlay(ORBIT_1); // ORBIT_1
  ROTATE(); // Overlay ORBIT-OV
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xbd81: WORD 'SEEGR' codep=0x224c wordp=0xbd8b
// ================================================

void SEEGR() // SEEGR
{
  LoadOverlay(ORBIT_1); // ORBIT_1
  SEEGRID(); // Overlay ORBIT-OV
}


// ================================================
// 0xbd91: WORD 'MROTA' codep=0x224c wordp=0xbd9b params=0 returns=0
// ================================================

void MROTA() // MROTA
{
  Push(0xbd79); // 'ROTAT'
  MODULE(); // MODULE
}


// ================================================
// 0xbda3: WORD 'CULTURE' codep=0x1ab5 wordp=0xbdaf
// ================================================
// 0xbdaf: db 0xbd 0xbd 0x0b 0xbd 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '                    '

// ================================================
// 0xbdc3: WORD 'CULTUR' codep=0x83dd wordp=0xbdce
// ================================================
// Overlay CULTUR = 0x6ca2

// ================================================
// 0xbdd0: WORD '.CULT' codep=0x224c wordp=0xbdda
// ================================================

void DrawCULT() // .CULT
{
  LoadOverlay(CULTUR); // CULTUR
  DrawCULTURE(); // Overlay CULTURE
}


// ================================================
// 0xbde0: WORD '.RACE' codep=0x224c wordp=0xbdea
// ================================================

void DrawRACE() // .RACE
{
  LoadOverlay(CULTUR); // CULTUR
  DrawRACE(); // Overlay CULTURE
}


// ================================================
// 0xbdf0: WORD 'DESCRI' codep=0x1ab5 wordp=0xbdfb
// ================================================
// 0xbdfb: db 0x09 0xbe 0xb1 0xbd 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '                    '

// ================================================
// 0xbe0f: WORD 'DESCR' codep=0x83dd wordp=0xbe19
// ================================================
// Overlay DESCR = 0x7923

// ================================================
// 0xbe1b: WORD 'DO-LO' codep=0x224c wordp=0xbe25
// ================================================

void DO_dash_LO() // DO-LO
{
  LoadOverlay(DESCR); // DESCR
  DESCRIBE(); // Overlay DESCRIBE
}


// ================================================
// 0xbe2b: WORD '.LIFE' codep=0x224c wordp=0xbe35
// ================================================

void DrawLIFE() // .LIFE
{
  LoadOverlay(DESCR); // DESCR
  CLASS_gt_TY(); // Overlay DESCRIBE
}


// ================================================
// 0xbe3b: WORD '.HUFF' codep=0x224c wordp=0xbe45
// ================================================

void DrawHUFF() // .HUFF
{
  LoadOverlay(DESCR); // DESCR
  DrawPHRASE(); // Overlay DESCRIBE
}


// ================================================
// 0xbe4b: WORD '.AHUF' codep=0x224c wordp=0xbe55
// ================================================

void DrawAHUF() // .AHUF
{
  LoadOverlay(DESCR); // DESCR
  DrawAPHRASE(); // Overlay DESCRIBE
}


// ================================================
// 0xbe5b: WORD 'SSCN' codep=0x224c wordp=0xbe64
// ================================================

void SSCN() // SSCN
{
  LoadOverlay(DESCR); // DESCR
  SYSCAN(); // Overlay DESCRIBE
}


// ================================================
// 0xbe6a: WORD '?COMS' codep=0x224c wordp=0xbe74
// ================================================

void IsCOMS() // ?COMS
{
  LoadOverlay(DESCR); // DESCR
  IsCOMSG(); // Overlay DESCRIBE
}


// ================================================
// 0xbe7a: WORD 'SSCAN' codep=0x224c wordp=0xbe84 params=0 returns=0
// ================================================

void SSCAN() // SSCAN
{
  Push(0xbe64); // 'SSCN'
  MODULE(); // MODULE
}


// ================================================
// 0xbe8c: WORD 'CPAUS' codep=0x224c wordp=0xbe96
// ================================================

void CPAUS() // CPAUS
{
  LoadOverlay(DESCR); // DESCR
  APAUSE(); // Overlay DESCRIBE
}


// ================================================
// 0xbe9c: WORD 'COMM-VOC' codep=0x1ab5 wordp=0xbea9
// ================================================
// 0xbea9: db 0xb7 0xbe 0xfd 0xbd 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '                    '

// ================================================
// 0xbebd: WORD 'COMM-OV' codep=0x83dd wordp=0xbec9
// ================================================
// Overlay COMM_dash_OV = 0x7b03

// ================================================
// 0xbecb: WORD 'OVINIT-_2' codep=0x224c wordp=0xbed7
// ================================================

void OVINIT_dash__2() // OVINIT-_2
{
  LoadOverlay(COMM_dash_OV); // COMM-OV
  ICOMM(); // Overlay COMM-OV
  SAVE_dash_OV(); // SAVE-OV
  Push(1);
  Push(0xb822); // 'PSYCH'
  MODULE(); // MODULE
}


// ================================================
// 0xbee7: WORD 'U-COMM' codep=0x224c wordp=0xbef2
// ================================================

void U_dash_COMM() // U-COMM
{
  LoadOverlay(COMM_dash_OV); // COMM-OV
  CL1(); // Overlay COMM-OV
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xbefa: WORD 'COMM-EX' codep=0x224c wordp=0xbf06
// ================================================

void COMM_dash_EX() // COMM-EX
{
  LoadOverlay(COMM_dash_OV); // COMM-OV
  COMM_dash_EXPERT(); // Overlay COMM-OV
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xbf0e: WORD 'A>O' codep=0x224c wordp=0xbf16
// ================================================

void A_gt_O() // A>O
{
  LoadOverlay(COMM_dash_OV); // COMM-OV
  A_gt_ORIGINATO(); // Overlay COMM-OV
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xbf1e: WORD 'O>C' codep=0x224c wordp=0xbf26
// ================================================

void O_gt_C() // O>C
{
  LoadOverlay(COMM_dash_OV); // COMM-OV
  ORIGINATOR_gt_(); // Overlay COMM-OV
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xbf2e: WORD '>HAIL' codep=0x224c wordp=0xbf38
// ================================================

void _gt_HAIL() // >HAIL
{
  LoadOverlay(COMM_dash_OV); // COMM-OV
  _gt_HAIL_do_(); // Overlay COMM-OV
}


// ================================================
// 0xbf3e: WORD 'OV+!EDL' codep=0x224c wordp=0xbf4a
// ================================================

void OV_plus__ex_EDL() // OV+!EDL
{
  LoadOverlay(COMM_dash_OV); // COMM-OV
  _plus__ex_EDL(); // Overlay COMM-OV
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xbf52: WORD 'OVEDL+A' codep=0x224c wordp=0xbf5e
// ================================================

void OVEDL_plus_A() // OVEDL+A
{
  LoadOverlay(COMM_dash_OV); // COMM-OV
  EDL_plus_AUX(); // Overlay COMM-OV
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xbf66: WORD 'P>CT' codep=0x224c wordp=0xbf6f
// ================================================

void P_gt_CT() // P>CT
{
  LoadOverlay(COMM_dash_OV); // COMM-OV
  _ro_PHRASE_gt_CT_rc_(); // Overlay COMM-OV
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xbf77: WORD 'CTUP' codep=0x224c wordp=0xbf80
// ================================================

void CTUP() // CTUP
{
  LoadOverlay(COMM_dash_OV); // COMM-OV
  CTSCROLL(); // Overlay COMM-OV
}


// ================================================
// 0xbf86: WORD '1SYL' codep=0x224c wordp=0xbf8f
// ================================================

void _1SYL() // 1SYL
{
  LoadOverlay(COMM_dash_OV); // COMM-OV
  _1SYL(); // Overlay COMM-OV
}


// ================================================
// 0xbf95: WORD 'MCOMM-E' codep=0x224c wordp=0xbfa1 params=0 returns=0
// ================================================

void MCOMM_dash_E() // MCOMM-E
{
  Push(0xbf06); // 'COMM-EX'
  MODULE(); // MODULE
}


// ================================================
// 0xbfa9: WORD 'MEDL+AU' codep=0x224c wordp=0xbfb5 params=0 returns=0
// ================================================

void MEDL_plus_AU() // MEDL+AU
{
  Push(0xbf5e); // 'OVEDL+A'
  MODULE(); // MODULE
}


// ================================================
// 0xbfbd: WORD 'DA-STR' codep=0x224c wordp=0xbfc8
// ================================================

void DA_dash_STR() // DA-STR
{
  LoadOverlay(COMM_dash_OV); // COMM-OV
  IAPWR(); // Overlay COMM-OV
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xbfd0: WORD 'EDL>P' codep=0x224c wordp=0xbfda
// ================================================

void EDL_gt_P() // EDL>P
{
  LoadOverlay(COMM_dash_OV); // COMM-OV
  EDL_gt_P(); // Overlay COMM-OV
}


// ================================================
// 0xbfe0: WORD 'BLDLI-VOC' codep=0x1ab5 wordp=0xbfea
// ================================================
// 0xbfea: db 0xf8 0xbf 0xab 0xbe 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '                    '

// ================================================
// 0xbffe: WORD 'BLDLI-OV' codep=0x83dd wordp=0xc008
// ================================================
// Overlay BLDLI_dash_OV = 0x91c3

// ================================================
// 0xc00a: WORD 'BLDLI' codep=0x224c wordp=0xc014
// ================================================

void BLDLI() // BLDLI
{
  LoadOverlay(BLDLI_dash_OV); // BLDLI-OV
  BLD_dash_LIST(); // Overlay BLDLISTS
}


// ================================================
// 0xc01a: WORD 'SET-C' codep=0x224c wordp=0xc024
// ================================================

void SET_dash_C() // SET-C
{
  LoadOverlay(BLDLI_dash_OV); // BLDLI-OV
  SET_dash_CARG(); // Overlay BLDLISTS
}


// ================================================
// 0xc02a: WORD 'T>O' codep=0x224c wordp=0xc032
// ================================================

void T_gt_O() // T>O
{
  LoadOverlay(BLDLI_dash_OV); // BLDLI-OV
  TRADE_gt_OR(); // Overlay BLDLISTS
}


// ================================================
// 0xc038: WORD '!TAMT' codep=0x224c wordp=0xc042
// ================================================

void StoreTAMT() // !TAMT
{
  LoadOverlay(BLDLI_dash_OV); // BLDLI-OV
  StoreTAMT(); // Overlay BLDLISTS
}


// ================================================
// 0xc048: WORD '@TAMT' codep=0x224c wordp=0xc052
// ================================================

void GetTAMT() // @TAMT
{
  LoadOverlay(BLDLI_dash_OV); // BLDLI-OV
  GetTAMT(); // Overlay BLDLISTS
}


// ================================================
// 0xc058: WORD 'SCI-VOC' codep=0x1ab5 wordp=0xc064
// ================================================
// 0xc064: db 0x72 0xc0 0xec 0xbf 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 'r                   '

// ================================================
// 0xc078: WORD 'SCI-OV' codep=0x83dd wordp=0xc083
// ================================================
// Overlay SCI_dash_OV = 0x82e5

// ================================================
// 0xc085: WORD 'OV/STA' codep=0x224c wordp=0xc090
// ================================================

void OV_slash_STA() // OV/STA
{
  LoadOverlay(SCI_dash_OV); // SCI-OV
  _ro__slash_STATUS_rc_(); // Overlay SCIENCE
}


// ================================================
// 0xc096: WORD 'OV/STX' codep=0x224c wordp=0xc0a1
// ================================================

void OV_slash_STX() // OV/STX
{
  LoadOverlay(SCI_dash_OV); // SCI-OV
  DrawSTAT_dash_TEXT(); // Overlay SCIENCE
}


// ================================================
// 0xc0a7: WORD 'OVSTAR' codep=0x224c wordp=0xc0b2
// ================================================

void OVSTAR() // OVSTAR
{
  LoadOverlay(SCI_dash_OV); // SCI-OV
  Push(0);
  DrawSTARDATE(); // Overlay SCIENCE
}


// ================================================
// 0xc0ba: WORD 'OV/SEN' codep=0x224c wordp=0xc0c5
// ================================================

void OV_slash_SEN() // OV/SEN
{
  LoadOverlay(SCI_dash_OV); // SCI-OV
  _ro__slash_SENSORS_rc_(); // Overlay SCIENCE
}


// ================================================
// 0xc0cb: WORD '?HEAVY' codep=0x224c wordp=0xc0d6
// ================================================

void IsHEAVY() // ?HEAVY
{
  LoadOverlay(SCI_dash_OV); // SCI-OV
  IsHEAVY(); // Overlay SCIENCE
}


// ================================================
// 0xc0dc: WORD '/STARD' codep=0x224c wordp=0xc0e7 params=0 returns=0
// ================================================

void _slash_STARD() // /STARD
{
  Push(0xc0b2); // 'OVSTAR'
  MODULE(); // MODULE
}


// ================================================
// 0xc0ef: WORD '.SORD' codep=0x224c wordp=0xc0f9 params=0 returns=0
// ================================================

void DrawSORD() // .SORD
{
  Push(Read16(pp__n_AUX)); // #AUX @
  Push(Read16(regsp)); // DUP
  Push(Pop()==1?1:0); //  1 =
  if (Pop() != 0)
  {
    Push(0xc0a1); // 'OV/STX'
    MODULE(); // MODULE
  }
  Push(Pop()==3?1:0); //  3 =
  if (Pop() == 0) return;
  Push(0xbcb8); // 'OVDBA'
  MODULE(); // MODULE
}


// ================================================
// 0xc11d: WORD 'HEALTI' codep=0x1d29 wordp=0xc128
// ================================================
// 0xc128: db 0x20 0x45 ' E'

// ================================================
// 0xc12a: WORD 'LASTAP' codep=0x1d29 wordp=0xc135
// ================================================
// 0xc135: db 0x49 0x4e 0x54 0x20 'INT '

// ================================================
// 0xc139: WORD 'ROSTER' codep=0x1d29 wordp=0xc144
// ================================================
// 0xc144: db 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 '                  '

// ================================================
// 0xc156: WORD 'HEALER' codep=0x1ab5 wordp=0xc161
// ================================================
// 0xc161: db 0x6f 0xc1 0x66 0xc0 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 'o f                 '

// ================================================
// 0xc175: WORD 'HEAL-O' codep=0x83dd wordp=0xc180
// ================================================
// Overlay HEAL_dash_O = 0x8c24

// ================================================
// 0xc182: WORD '(OBI' codep=0x224c wordp=0xc18b
// ================================================

void _ro_OBI() // (OBI
{
  LoadOverlay(HEAL_dash_O); // HEAL-O
  OBITS(); // Overlay HEAL-OV
}


// ================================================
// 0xc191: WORD '(.VI' codep=0x224c wordp=0xc19a
// ================================================

void _ro__dot_VI() // (.VI
{
  LoadOverlay(HEAL_dash_O); // HEAL-O
  DrawVITS(); // Overlay HEAL-OV
}


// ================================================
// 0xc1a0: WORD '(HEA' codep=0x224c wordp=0xc1a9
// ================================================

void _ro_HEA() // (HEA
{
  LoadOverlay(HEAL_dash_O); // HEAL-O
  HEAL(); // Overlay HEAL-OV
}


// ================================================
// 0xc1af: WORD '(ROL' codep=0x224c wordp=0xc1b8
// ================================================

void _ro_ROL() // (ROL
{
  LoadOverlay(HEAL_dash_O); // HEAL-O
  ROLE_dash_C(); // Overlay HEAL-OV
}


// ================================================
// 0xc1be: WORD 'HEAL' codep=0x224c wordp=0xc1c7 params=0 returns=0
// ================================================

void HEAL() // HEAL
{
  Push(0xc1a9); // '(HEA'
  MODULE(); // MODULE
}


// ================================================
// 0xc1cf: WORD '.VIT' codep=0x224c wordp=0xc1d8 params=0 returns=0
// ================================================

void DrawVIT() // .VIT
{
  Push(0xc19a); // '(.VI'
  MODULE(); // MODULE
}


// ================================================
// 0xc1e0: WORD 'KILL' codep=0x224c wordp=0xc1e9
// ================================================

void KILL() // KILL
{
  LoadOverlay(HEAL_dash_O); // HEAL-O
  KILL(); // Overlay HEAL-OV
}


// ================================================
// 0xc1ef: WORD '?APP' codep=0x224c wordp=0xc1f8 params=0 returns=1
// ================================================

void IsAPP() // ?APP
{
  TIME(); // TIME
  _2_at_(); // 2@
  Push(pp_LASTAP); // LASTAP
  _2_at_(); // 2@
  Push(Read16(pp_HEALTI)); // HEALTI @
  Push(0);
  D_plus_(); // D+
  D_gt_(); // D>
  Push(Pop() & Read16(pp_IsHEAL)); //  ?HEAL @ AND
  if (Pop() != 0)
  {
    TIME(); // TIME
    _2_at_(); // 2@
    Push(pp_LASTAP); // LASTAP
    _2_ex__2(); // 2!_2
    Push(1);
    return;
  }
  Push(0);
}


// ================================================
// 0xc226: WORD 'BANK-VOC' codep=0x1ab5 wordp=0xc233
// ================================================
// 0xc233: db 0x41 0xc2 0x63 0xc1 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 'A c                 '

// ================================================
// 0xc247: WORD 'BANK-OV' codep=0x83dd wordp=0xc253
// ================================================
// Overlay BANK_dash_OV = 0x730f

// ================================================
// 0xc255: WORD 'OVINIT-_1' codep=0x224c wordp=0xc261
// ================================================

void OVINIT_dash__1() // OVINIT-_1
{
  LoadOverlay(BANK_dash_OV); // BANK-OV
  INIT_dash_BALANCE(); // Overlay BANK-OV
}


// ================================================
// 0xc267: WORD 'OVTRANS' codep=0x224c wordp=0xc273
// ================================================

void OVTRANS() // OVTRANS
{
  LoadOverlay(BANK_dash_OV); // BANK-OV
  TRANSACT(); // Overlay BANK-OV
}


// ================================================
// 0xc279: WORD 'OVD@BAL' codep=0x224c wordp=0xc285
// ================================================

void OVD_at_BAL() // OVD@BAL
{
  LoadOverlay(BANK_dash_OV); // BANK-OV
  GetDBALANCE(); // Overlay BANK-OV
}


// ================================================
// 0xc28b: WORD 'OVD!BAL' codep=0x224c wordp=0xc297
// ================================================

void OVD_ex_BAL() // OVD!BAL
{
  LoadOverlay(BANK_dash_OV); // BANK-OV
  StoreDBALANCE(); // Overlay BANK-OV
}


// ================================================
// 0xc29d: WORD 'OV?BALA' codep=0x224c wordp=0xc2a9
// ================================================

void OV_ask_BALA() // OV?BALA
{
  LoadOverlay(BANK_dash_OV); // BANK-OV
  IsBALANCE(); // Overlay BANK-OV
}


// ================================================
// 0xc2af: WORD 'OV!TFLA' codep=0x224c wordp=0xc2bb
// ================================================

void OV_ex_TFLA() // OV!TFLA
{
  LoadOverlay(BANK_dash_OV); // BANK-OV
  StoreTFLAG(); // Overlay BANK-OV
}


// ================================================
// 0xc2c1: WORD 'I-TRANS' codep=0x224c wordp=0xc2cd
// ================================================

void I_dash_TRANS() // I-TRANS
{
  LoadOverlay(BANK_dash_OV); // BANK-OV
  INT_pe_(); // Overlay BANK-OV
}


// ================================================
// 0xc2d3: WORD 'U-B' codep=0x224c wordp=0xc2db
// ================================================

void U_dash_B() // U-B
{
  LoadOverlay(BANK_dash_OV); // BANK-OV
  _ro_U_dash_BANK_rc_(); // Overlay BANK-OV
}


// ================================================
// 0xc2e1: WORD 'T+BALAN' codep=0x224c wordp=0xc2ed
// ================================================

void T_plus_BALAN() // T+BALAN
{
  LoadOverlay(BANK_dash_OV); // BANK-OV
  GetDBALANCE(); // Overlay BANK-OV
  D_plus_(); // D+
  StoreDBALANCE(); // Overlay BANK-OV
  StoreTFLAG(); // Overlay BANK-OV
}


// ================================================
// 0xc2f9: WORD 'XCOM-VOC' codep=0x1ab5 wordp=0xc306
// ================================================
// 0xc306: db 0x14 0xc3 0x35 0xc2 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '  5                 '

// ================================================
// 0xc31a: WORD 'XCOM-OV' codep=0x83dd wordp=0xc326
// ================================================
// Overlay XCOM_dash_OV = 0x7cdb

// ================================================
// 0xc328: WORD 'XCOMM' codep=0x224c wordp=0xc332
// ================================================

void XCOMM() // XCOMM
{
  LoadOverlay(XCOM_dash_OV); // XCOM-OV
  _ro_XCOMM_rc_(); // Overlay COMMSPEC-OV
}


// ================================================
// 0xc338: WORD '#>PRO' codep=0x224c wordp=0xc342
// ================================================

void _n__gt_PRO() // #>PRO
{
  LoadOverlay(XCOM_dash_OV); // XCOM-OV
  _n__gt_PRO(); // Overlay COMMSPEC-OV
}


// ================================================
// 0xc348: WORD 'REPAIR' codep=0x1ab5 wordp=0xc353
// ================================================
// 0xc353: db 0x61 0xc3 0x08 0xc3 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 'a                   '

// ================================================
// 0xc367: WORD 'REPAIR-' codep=0x83dd wordp=0xc373
// ================================================
// Overlay REPAIR_dash_ = 0x8c73

// ================================================
// 0xc375: WORD 'DO-REPA' codep=0x224c wordp=0xc381
// ================================================

void DO_dash_REPA() // DO-REPA
{
  LoadOverlay(REPAIR_dash_); // REPAIR-
  DO_dash_REPA(); // Overlay REPAIR-OV
  SAVE_dash_OV(); // SAVE-OV
  TIME(); // TIME
  _2_at_(); // 2@
  Push(pp_LASTREP); // LASTREP
  _2_ex__2(); // 2!_2
}


// ================================================
// 0xc391: WORD '?SHIP-R' codep=0x224c wordp=0xc39d params=0 returns=1
// ================================================

void IsSHIP_dash_R() // ?SHIP-R
{
  TIME(); // TIME
  _2_at_(); // 2@
  Push(pp_LASTREP); // LASTREP
  _2_at_(); // 2@
  D_dash_(); // D-
  DABS(); // DABS
  Push(Read16(pp_REPAIRT)); // REPAIRT @
  Push(0);
  D_gt_(); // D>
  Push(Pop() & Read16(pp_IsREPAIR) | (Read16(pp_IsREPAIR)==1?1:0)); //  ?REPAIR @ AND ?REPAIR @ 1 = OR
}


// ================================================
// 0xc3c3: WORD 'MREPAIR' codep=0x224c wordp=0xc3cf params=0 returns=0
// ================================================

void MREPAIR() // MREPAIR
{
  IsSHIP_dash_R(); // ?SHIP-R
  if (Pop() == 0) return;
  Push(0xc381); // 'DO-REPA'
  MODULE(); // MODULE
  Push(Read16(pp__n_AUX)==3?1:0); // #AUX @ 3 =
  if (Pop() == 0) return;
  Push(0xbcb8); // 'OVDBA'
  MODULE(); // MODULE
}


// ================================================
// 0xc3ef: WORD 'BARTE' codep=0x1ab5 wordp=0xc3f9
// ================================================
// 0xc3f9: db 0x07 0xc4 0x55 0xc3 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '  U                 '

// ================================================
// 0xc40d: WORD 'BARTER-' codep=0x83dd wordp=0xc419
// ================================================
// Overlay BARTER_dash_ = 0x9262

// ================================================
// 0xc41b: WORD 'DOENTER' codep=0x224c wordp=0xc427
// ================================================

void DOENTER() // DOENTER
{
  LoadOverlay(BARTER_dash_); // BARTER-
  DO_dash_ENTER(); // Overlay BARTER
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xc42f: WORD 'DOOPEN' codep=0x224c wordp=0xc43a
// ================================================

void DOOPEN() // DOOPEN
{
  LoadOverlay(BARTER_dash_); // BARTER-
  DO_dash_OPEN_2(); // Overlay BARTER
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xc442: WORD 'DOBARTE' codep=0x224c wordp=0xc44e
// ================================================

void DOBARTE() // DOBARTE
{
  LoadOverlay(BARTER_dash_); // BARTER-
  DO_dash_BARTER(); // Overlay BARTER
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xc456: WORD '!PFLAGS' codep=0x224c wordp=0xc462
// ================================================

void StorePFLAGS() // !PFLAGS
{
  LoadOverlay(BARTER_dash_); // BARTER-
  StorePFLAGS(); // Overlay BARTER
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xc46a: WORD 'L>P' codep=0x224c wordp=0xc472
// ================================================

void L_gt_P() // L>P
{
  LoadOverlay(BARTER_dash_); // BARTER-
  L_gt_P(); // Overlay BARTER
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xc47a: WORD 'MAR' codep=0x1ab5 wordp=0xc482
// ================================================
// 0xc482: db 0x90 0xc4 0xfb 0xc3 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '                    '

// ================================================
// 0xc496: WORD 'MARKE' codep=0x83dd wordp=0xc4a0
// ================================================
// Overlay MARKE = 0x93bc

// ================================================
// 0xc4a2: WORD 'TRADE' codep=0x224c wordp=0xc4ac
// ================================================

void TRADE() // TRADE
{
  LoadOverlay(MARKE); // MARKE
  TRADE(); // Overlay MARKET
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xc4b4: WORD 'SITE-VOC' codep=0x1ab5 wordp=0xc4c1
// ================================================
// 0xc4c1: db 0xcf 0xc4 0x84 0xc4 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '                    '

// ================================================
// 0xc4d5: WORD 'SITE-OV' codep=0x83dd wordp=0xc4e1
// ================================================
// Overlay SITE_dash_OV = 0x7111

// ================================================
// 0xc4e3: WORD '(.MERC)' codep=0x224c wordp=0xc4ef
// ================================================

void _ro__dot_MERC_rc_() // (.MERC)
{
  LoadOverlay(SITE_dash_OV); // SITE-OV
  DrawMERCATOR(); // Overlay SITE-OV
  OV_dash_CANC(); // OV-CANC
}


// ================================================
// 0xc4f7: WORD '(GETSITE)' codep=0x224c wordp=0xc505
// ================================================

void _ro_GETSITE_rc_() // (GETSITE)
{
  LoadOverlay(SITE_dash_OV); // SITE-OV
  GETSITE_2(); // Overlay SITE-OV
  SAVE_dash_OV(); // SAVE-OV
  OV_dash_CANC(); // OV-CANC
}


// ================================================
// 0xc50f: WORD 'FLUX-VOC' codep=0x1ab5 wordp=0xc51c
// ================================================
// 0xc51c: db 0x2a 0xc5 0xc3 0xc4 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '*                   '

// ================================================
// 0xc530: WORD 'FLUX-OV' codep=0x83dd wordp=0xc53c
// ================================================
// Overlay FLUX_dash_OV = 0x88d6

// ================================================
// 0xc53e: WORD 'OVFLU' codep=0x224c wordp=0xc548
// ================================================

void OVFLU() // OVFLU
{
  LoadOverlay(FLUX_dash_OV); // FLUX-OV
  FLUX(); // Overlay FLUX-EFFECT
}


// ================================================
// 0xc54e: WORD 'JUMPF' codep=0x224c wordp=0xc558
// ================================================

void JUMPF() // JUMPF
{
  LoadOverlay(FLUX_dash_OV); // FLUX-OV
  JUMPFX(); // Overlay FLUX-EFFECT
}


// ================================================
// 0xc55e: WORD 'DPART-VOC' codep=0x1ab5 wordp=0xc56c
// ================================================
// 0xc56c: db 0x7a 0xc5 0x1e 0xc5 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 'z                   '

// ================================================
// 0xc580: WORD 'DPART-OV' codep=0x83dd wordp=0xc58d
// ================================================
// Overlay DPART_dash_OV = 0x6edf

// ================================================
// 0xc58f: WORD 'DEPART' codep=0x224c wordp=0xc59a
// ================================================

void DEPART() // DEPART
{
  LoadOverlay(DPART_dash_OV); // DPART-OV
  DEPART(); // Overlay DEPART
}


// ================================================
// 0xc5a0: WORD 'CLOUD-V' codep=0x1ab5 wordp=0xc5ac
// ================================================
// 0xc5ac: db 0xba 0xc5 0x6e 0xc5 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '  n                 '

// ================================================
// 0xc5c0: WORD 'CLOUD-O' codep=0x83dd wordp=0xc5cc
// ================================================
// Overlay CLOUD_dash_O = 0x9512

// ================================================
// 0xc5ce: WORD 'DO-CLOU' codep=0x224c wordp=0xc5da
// ================================================

void DO_dash_CLOU() // DO-CLOU
{
  LoadOverlay(CLOUD_dash_O); // CLOUD-O
  DO_dash_CLOUD(); // Overlay CLOUD-OV
}


// ================================================
// 0xc5e0: WORD 'NAV-VOC' codep=0x1ab5 wordp=0xc5ec
// ================================================
// 0xc5ec: db 0xfa 0xc5 0xae 0xc5 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '                    '

// ================================================
// 0xc600: WORD 'NAV-O' codep=0x83dd wordp=0xc60a
// ================================================
// Overlay NAV_dash_O = 0x8442

// ================================================
// 0xc60c: WORD 'OV/(D' codep=0x224c wordp=0xc616
// ================================================

void OV_slash__ro_D() // OV/(D
{
  LoadOverlay(NAV_dash_O); // NAV-O
  _ro__slash__ro_DIS_rc_ARM_rc_(); // Overlay NAVIGATR
}


// ================================================
// 0xc61c: WORD 'OV/(U' codep=0x224c wordp=0xc626
// ================================================

void OV_slash__ro_U() // OV/(U
{
  LoadOverlay(NAV_dash_O); // NAV-O
  _ro__slash__ro_UD_rc_SHIELD_rc_(); // Overlay NAVIGATR
}


// ================================================
// 0xc62c: WORD 'OV-SH' codep=0x224c wordp=0xc636
// ================================================

void OV_dash_SH() // OV-SH
{
  CTINIT(); // CTINIT
  LoadOverlay(NAV_dash_O); // NAV-O
  Push2Words("*SHIP");
  _gt_C_plus_S(); // >C+S
  _gt_DOWN_dash_SHIELD(); // Overlay NAVIGATR
  ICLOSE(); // ICLOSE
}


// ================================================
// 0xc644: WORD 'OV-AR' codep=0x224c wordp=0xc64e
// ================================================

void OV_dash_AR() // OV-AR
{
  CTINIT(); // CTINIT
  LoadOverlay(NAV_dash_O); // NAV-O
  Push2Words("*SHIP");
  _gt_C_plus_S(); // >C+S
  _gt_DISARM(); // Overlay NAVIGATR
  ICLOSE(); // ICLOSE
}


// ================================================
// 0xc65c: WORD 'DAMAGE-VOC' codep=0x1ab5 wordp=0xc668
// ================================================
// 0xc668: db 0x76 0xc6 0xee 0xc5 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 'v                   '

// ================================================
// 0xc67c: WORD 'DAMAGE-OV' codep=0x83dd wordp=0xc688
// ================================================
// Overlay DAMAGE_dash_OV = 0x8faf

// ================================================
// 0xc68a: WORD 'DO-DA' codep=0x224c wordp=0xc694
// ================================================

void DO_dash_DA() // DO-DA
{
  LoadOverlay(DAMAGE_dash_OV); // DAMAGE-OV
  DO_dash_DAMA(); // Overlay DAMAGE-OV
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xc69c: WORD '>DAM' codep=0x224c wordp=0xc6a5
// ================================================

void _gt_DAM() // >DAM
{
  LoadOverlay(DAMAGE_dash_OV); // DAMAGE-OV
  _n_WOUNDE(); // Overlay DAMAGE-OV
  _099(); // 099
  DO_dash_ADAM(); // Overlay DAMAGE-OV
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xc6b1: WORD '.AUX' codep=0x224c wordp=0xc6ba
// ================================================

void DrawAUX() // .AUX
{
  LoadOverlay(DAMAGE_dash_OV); // DAMAGE-OV
  REFRESH(); // Overlay DAMAGE-OV
}


// ================================================
// 0xc6c0: WORD 'CREWD' codep=0x224c wordp=0xc6ca
// ================================================

void CREWD() // CREWD
{
  LoadOverlay(DAMAGE_dash_OV); // DAMAGE-OV
  CDAM(); // Overlay DAMAGE-OV
}


// ================================================
// 0xc6d0: WORD 'BLST' codep=0x224c wordp=0xc6d9
// ================================================

void BLST() // BLST
{
  LoadOverlay(DAMAGE_dash_OV); // DAMAGE-OV
  DOBLAST(); // Overlay DAMAGE-OV
}


// ================================================
// 0xc6df: WORD '#MISS' codep=0x1d29 wordp=0xc6e9
// ================================================
// 0xc6e9: db 0x00 0x00 '  '

// ================================================
// 0xc6eb: WORD 'WEAP' codep=0x1ab5 wordp=0xc6f4
// ================================================
// 0xc6f4: db 0x02 0xc7 0x6a 0xc6 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '  j                 '

// ================================================
// 0xc708: WORD 'WEA' codep=0x83dd wordp=0xc710
// ================================================
// Overlay WEA = 0x9668

// ================================================
// 0xc712: WORD 'TAR' codep=0x224c wordp=0xc71a
// ================================================

void TAR() // TAR
{
  LoadOverlay(WEA); // WEA
  TARG(); // Overlay WEAPONS
}


// ================================================
// 0xc720: WORD 'DTA_2' codep=0x224c wordp=0xc728 params=0 returns=0
// ================================================

void DTA_2() // DTA_2
{
  Push(0xc71a); // 'TAR'
  MODULE(); // MODULE
}


// ================================================
// 0xc730: WORD 'DNL_2' codep=0x224c wordp=0xc738
// ================================================

void DNL_2() // DNL_2
{
  LoadOverlay(WEA); // WEA
  DONULL(); // Overlay WEAPONS
}


// ================================================
// 0xc73e: WORD 'GNL' codep=0x224c wordp=0xc746
// ================================================

void GNL() // GNL
{
  LoadOverlay(WEA); // WEA
  GNUNK_gt_NUL(); // Overlay WEAPONS
}


// ================================================
// 0xc74c: WORD 'DNL_1' codep=0x224c wordp=0xc754 params=0 returns=0
// ================================================

void DNL_1() // DNL_1
{
  Push(0xc738); // 'DNL_2'
  MODULE(); // MODULE
}


// ================================================
// 0xc75c: WORD 'EYE-VOC' codep=0x1ab5 wordp=0xc768
// ================================================
// 0xc768: db 0x76 0xc7 0xf6 0xc6 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 'v                   '

// ================================================
// 0xc77c: WORD 'EYE-O' codep=0x83dd wordp=0xc786
// ================================================
// Overlay EYE_dash_O = 0x7888

// ================================================
// 0xc788: WORD 'AI' codep=0x224c wordp=0xc78f
// ================================================

void AI() // AI
{
  LoadOverlay(EYE_dash_O); // EYE-O
  DO_dash_EYE(); // Overlay EYE-OV
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xc797: WORD 'COMBAU-VOC' codep=0x1ab5 wordp=0xc7a2
// ================================================
// 0xc7a2: db 0xb0 0xc7 0x6a 0xc7 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '  j                 '

// ================================================
// 0xc7b6: WORD 'COMBAU-OV' codep=0x83dd wordp=0xc7c1
// ================================================
// Overlay COMBAU_dash_OV = 0x912a

// ================================================
// 0xc7c3: WORD 'CSCALE' codep=0x224c wordp=0xc7ce
// ================================================

void CSCALE() // CSCALE
{
  LoadOverlay(COMBAU_dash_OV); // COMBAU-OV
  SET_dash_SC(); // Overlay COMBAUX
}


// ================================================
// 0xc7d4: WORD '@NF' codep=0x224c wordp=0xc7dc
// ================================================

void GetNF() // @NF
{
  LoadOverlay(COMBAU_dash_OV); // COMBAU-OV
  GetNF(); // Overlay COMBAUX
}


// ================================================
// 0xc7e2: WORD 'COMBAT-VOC' codep=0x1ab5 wordp=0xc7f1
// ================================================
// 0xc7f1: db 0xff 0xc7 0xa4 0xc7 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '                    '

// ================================================
// 0xc805: WORD 'COMBAT-O' codep=0x83dd wordp=0xc812
// ================================================
// Overlay COMBAT_dash_O = 0x8e29

// ================================================
// 0xc814: WORD '(WAR' codep=0x224c wordp=0xc81d
// ================================================

void _ro_WAR() // (WAR
{
  LoadOverlay(COMBAT_dash_O); // COMBAT-O
  WAR(); // Overlay COMBAT-OV
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xc825: WORD '(COM' codep=0x224c wordp=0xc82e
// ================================================

void _ro_COM() // (COM
{
  LoadOverlay(COMBAT_dash_O); // COMBAT-O
  _ro_COMB_rc_(); // Overlay COMBAT-OV
}


// ================================================
// 0xc834: WORD 'C&C' codep=0x224c wordp=0xc83c
// ================================================

void C_and_C() // C&C
{
  LoadOverlay(COMBAT_dash_O); // COMBAT-O
  CLEAR_dash_CO(); // Overlay COMBAT-OV
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xc844: WORD '?J' codep=0x224c wordp=0xc84b
// ================================================

void IsJ() // ?J
{
  LoadOverlay(COMBAT_dash_O); // COMBAT-O
  IsJ(); // Overlay COMBAT-OV
}


// ================================================
// 0xc851: WORD '?JMP' codep=0x224c wordp=0xc85a params=0 returns=0
// ================================================

void IsJMP() // ?JMP
{
  Push(0xc84b); // '?J'
  MODULE(); // MODULE
}


// ================================================
// 0xc862: WORD 'COMB' codep=0x224c wordp=0xc86b params=0 returns=0
// ================================================

void COMB() // COMB
{
  Push(0xc82e); // '(COM'
  MODULE(); // MODULE
}


// ================================================
// 0xc873: WORD 'WAR' codep=0x224c wordp=0xc87b params=0 returns=0
// ================================================

void WAR() // WAR
{
  Push(0xc81d); // '(WAR'
  MODULE(); // MODULE
}


// ================================================
// 0xc883: WORD 'END-' codep=0x1ab5 wordp=0xc88c
// ================================================
// 0xc88c: db 0x9a 0xc8 0xf3 0xc7 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '                    '

// ================================================
// 0xc8a0: WORD 'ENDI' codep=0x83dd wordp=0xc8a9
// ================================================
// Overlay ENDI = 0x6d80

// ================================================
// 0xc8ab: WORD 'HALL' codep=0x224c wordp=0xc8b4
// ================================================

void HALL() // HALL
{
  LoadOverlay(ENDI); // ENDI
  DO_dash_HALLS(); // Overlay ENDING
}


// ================================================
// 0xc8ba: WORD '?FRE' codep=0x224c wordp=0xc8c3
// ================================================

void IsFRE() // ?FRE
{
  LoadOverlay(ENDI); // ENDI
  FREEB(); // Overlay ENDING
}


// ================================================
// 0xc8c9: WORD 'DHL' codep=0x224c wordp=0xc8d1
// ================================================

void DHL() // DHL
{
  LoadOverlay(ENDI); // ENDI
  DUHL(); // Overlay ENDING
}


// ================================================
// 0xc8d7: WORD 'DUHL' codep=0x224c wordp=0xc8e0 params=0 returns=0
// ================================================

void DUHL() // DUHL
{
  Push(0xc8d1); // 'DHL'
  MODULE(); // MODULE
}


// ================================================
// 0xc8e8: WORD 'BOLT' codep=0x224c wordp=0xc8f1
// ================================================

void BOLT() // BOLT
{
  LoadOverlay(ENDI); // ENDI
  BOLT(); // Overlay ENDING
}


// ================================================
// 0xc8f7: WORD 'IUHL' codep=0x224c wordp=0xc900
// ================================================

void IUHL() // IUHL
{
  LoadOverlay(ENDI); // ENDI
  StoreINIT(); // Overlay ENDING
}


// ================================================
// 0xc906: WORD 'HMISC-VOC' codep=0x1ab5 wordp=0xc914
// ================================================
// 0xc914: db 0x22 0xc9 0x8e 0xc8 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '"                   '

// ================================================
// 0xc928: WORD 'HMISC-O' codep=0x83dd wordp=0xc934
// ================================================
// Overlay HMISC_dash_O = 0x7153

// ================================================
// 0xc936: WORD 'UFM' codep=0x224c wordp=0xc93e
// ================================================

void UFM() // UFM
{
  LoadOverlay(HMISC_dash_O); // HMISC-O
  UF_dash_MESS(); // Overlay HYPERMSG-OV
}


// ================================================
// 0xc944: WORD 'USM' codep=0x224c wordp=0xc94c
// ================================================

void USM() // USM
{
  LoadOverlay(HMISC_dash_O); // HMISC-O
  US_dash_MESS(); // Overlay HYPERMSG-OV
}


// ================================================
// 0xc952: WORD 'NFM' codep=0x224c wordp=0xc95a
// ================================================

void NFM() // NFM
{
  LoadOverlay(HMISC_dash_O); // HMISC-O
  NF_dash_MESS(); // Overlay HYPERMSG-OV
}


// ================================================
// 0xc960: WORD 'NSM' codep=0x224c wordp=0xc968
// ================================================

void NSM() // NSM
{
  LoadOverlay(HMISC_dash_O); // HMISC-O
  NS_dash_MESS(); // Overlay HYPERMSG-OV
}


// ================================================
// 0xc96e: WORD '.EX' codep=0x224c wordp=0xc976
// ================================================

void DrawEX() // .EX
{
  LoadOverlay(HMISC_dash_O); // HMISC-O
  DrawEX(); // Overlay HYPERMSG-OV
}


// ================================================
// 0xc97c: WORD 'DMSG' codep=0x224c wordp=0xc985
// ================================================

void DMSG() // DMSG
{
  LoadOverlay(HMISC_dash_O); // HMISC-O
  DMSG(); // Overlay HYPERMSG-OV
}


// ================================================
// 0xc98b: WORD 'D-UP' codep=0x224c wordp=0xc994
// ================================================

void D_dash_UP() // D-UP
{
  LoadOverlay(HMISC_dash_O); // HMISC-O
  DISP_dash_SETUP(); // Overlay HYPERMSG-OV
}


// ================================================
// 0xc99a: WORD 'ORB>' codep=0x224c wordp=0xc9a3
// ================================================

void ORB_gt_() // ORB>
{
  LoadOverlay(HMISC_dash_O); // HMISC-O
  ORBITS_gt_(); // Overlay HYPERMSG-OV
}


// ================================================
// 0xc9a9: WORD '>ORB' codep=0x224c wordp=0xc9b2
// ================================================

void _gt_ORB() // >ORB
{
  LoadOverlay(HMISC_dash_O); // HMISC-O
  _gt_ORBITS(); // Overlay HYPERMSG-OV
}


// ================================================
// 0xc9b8: WORD 'OSET' codep=0x224c wordp=0xc9c1
// ================================================

void OSET() // OSET
{
  LoadOverlay(HMISC_dash_O); // HMISC-O
  OSET(); // Overlay HYPERMSG-OV
}


// ================================================
// 0xc9c7: WORD 'ORBUP' codep=0x224c wordp=0xc9d1 params=0 returns=0
// ================================================

void ORBUP() // ORBUP
{
  Push(0xc9c1); // 'OSET'
  MODULE(); // MODULE
}


// ================================================
// 0xc9d9: WORD 'TRAK-EN' codep=0x224c wordp=0xc9e5 params=0 returns=0
// ================================================

void TRAK_dash_EN() // TRAK-EN
{
  Push(Read16(pp_STAR_dash_HR) - Read16(pp_TRAK_dash_HR)); // STAR-HR @ TRAK-HR @ -
  ABS(); // ABS
  Push(2);
  Push(0x0017);
  WITHIN(); // WITHIN
  if (Pop() == 0) return;
  LoadOverlay(HMISC_dash_O); // HMISC-O
  TRAK_dash_E(); // Overlay HYPERMSG-OV
}


// ================================================
// 0xca03: WORD '!'EXT' codep=0x224c wordp=0xca0d
// ================================================

void Store_i_EXT() // !'EXT
{
  LoadOverlay(HMISC_dash_O); // HMISC-O
  _ro__ex__i_EXT_rc_(); // Overlay HYPERMSG-OV
}


// ================================================
// 0xca13: WORD '(CTXT)' codep=0x224c wordp=0xca1e
// ================================================

void _ro_CTXT_rc_() // (CTXT)
{
  LoadOverlay(HMISC_dash_O); // HMISC-O
  _ro_CTXT_rc_(); // Overlay HYPERMSG-OV
}


// ================================================
// 0xca24: WORD 'CONTEXT_2' codep=0x224c wordp=0xca30 params=0 returns=0
// ================================================

void CONTEXT_2() // CONTEXT_2
{
  Push(0xca1e); // '(CTXT)'
  MODULE(); // MODULE
}


// ================================================
// 0xca38: WORD '+A-VESS' codep=0x224c wordp=0xca44
// ================================================

void _plus_A_dash_VESS() // +A-VESS
{
  LoadOverlay(HMISC_dash_O); // HMISC-O
  _plus_A_dash_VESS(); // Overlay HYPERMSG-OV
}


// ================================================
// 0xca4a: WORD 'MTRAK-E' codep=0x224c wordp=0xca56 params=0 returns=0
// ================================================

void MTRAK_dash_E() // MTRAK-E
{
  Push(0xc9e5); // 'TRAK-EN'
  MODULE(); // MODULE
}


// ================================================
// 0xca5e: WORD 'SHPMOV-' codep=0x1ab5 wordp=0xca6a
// ================================================
// 0xca6a: db 0x78 0xca 0x16 0xc9 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 'x                   '

// ================================================
// 0xca7e: WORD 'HYPER' codep=0x83dd wordp=0xca88
// ================================================
// Overlay HYPER = 0x863a

// ================================================
// 0xca8a: WORD 'FLY' codep=0x224c wordp=0xca92
// ================================================

void FLY() // FLY
{
  LoadOverlay(HYPER); // HYPER
  FLY(); // Overlay HYPER-OV
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xca9a: WORD 'MAN' codep=0x224c wordp=0xcaa2
// ================================================

void MAN() // MAN
{
  LoadOverlay(HYPER); // HYPER
  CHK_dash_MOV(); // Overlay HYPER-OV
  if (Pop() != 0)
  {
    DrawAUXSYS(); // Overlay HYPER-OV
    Push(Read16(pp_SKIP2NE)); // SKIP2NE @
    if (Pop() != 0)
    {
      UNNEST(); // Overlay HYPER-OV
    } else
    {
      Push(0xf103); // 'CLEANUP'
      Push(0xefaf); // 'SET-DES'
      Push(0xf069); // 'SETUP-M'
      DOTASKS(SETUP_dash_M, SETUP_dash_M, SETUP_dash_M);
    }
    Push(pp_FTRIG); // FTRIG
    _099(); // 099
  }
  DrawSORD(); // .SORD
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xcad2: WORD '.AS' codep=0x224c wordp=0xcada
// ================================================

void DrawAS() // .AS
{
  LoadOverlay(HYPER); // HYPER
  DrawAUXSYS(); // Overlay HYPER-OV
}


// ================================================
// 0xcae0: WORD 'JMPSHP' codep=0x224c wordp=0xcaeb
// ================================================

void JMPSHP() // JMPSHP
{
  LoadOverlay(HYPER); // HYPER
  JMPSHP(); // Overlay HYPER-OV
}


// ================================================
// 0xcaf1: WORD 'UNNEST' codep=0x224c wordp=0xcafc
// ================================================

void UNNEST() // UNNEST
{
  LoadOverlay(HYPER); // HYPER
  UNNEST(); // Overlay HYPER-OV
}


// ================================================
// 0xcb02: WORD '>NEST' codep=0x224c wordp=0xcb0c
// ================================================

void _gt_NEST() // >NEST
{
  LoadOverlay(HYPER); // HYPER
  _plus_NEST(); // Overlay HYPER-OV
}


// ================================================
// 0xcb12: WORD 'GET-HA' codep=0x224c wordp=0xcb1d
// ================================================

void GET_dash_HA() // GET-HA
{
  LoadOverlay(HYPER); // HYPER
  _ro_GET_dash_AU(); // Overlay HYPER-OV
}


// ================================================
// 0xcb23: WORD 'OV.MVS' codep=0x224c wordp=0xcb2e
// ================================================

void OV_dot_MVS() // OV.MVS
{
  LoadOverlay(HYPER); // HYPER
  DrawMVS(); // Overlay HYPER-OV
}


// ================================================
// 0xcb34: WORD 'S>PORT' codep=0x224c wordp=0xcb3f
// ================================================

void S_gt_PORT() // S>PORT
{
  LoadOverlay(HYPER); // HYPER
  COME_dash_HI(); // Overlay HYPER-OV
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xcb47: WORD 'USE-E' codep=0x224c wordp=0xcb51
// ================================================

void USE_dash_E() // USE-E
{
  LoadOverlay(HYPER); // HYPER
  USE_dash_E(); // Overlay HYPER-OV
}


// ================================================
// 0xcb57: WORD 'MUSE-E' codep=0x224c wordp=0xcb62 params=0 returns=0
// ================================================

void MUSE_dash_E() // MUSE-E
{
  Push(0xcb51); // 'USE-E'
  MODULE(); // MODULE
}


// ================================================
// 0xcb6a: WORD 'ACR-VOC' codep=0x1ab5 wordp=0xcb72
// ================================================
// 0xcb72: db 0x80 0xcb 0x6c 0xca 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '  l                 '

// ================================================
// 0xcb86: WORD 'ACR-OV' codep=0x83dd wordp=0xcb8e
// ================================================
// Overlay ACR_dash_OV = 0x736d

// ================================================
// 0xcb90: WORD 'U-A' codep=0x224c wordp=0xcb98
// ================================================

void U_dash_A() // U-A
{
  LoadOverlay(ACR_dash_OV); // ACR-OV
  _ro_U_dash_ASSCREW_rc_(); // Overlay ASSCREW-OV
}


// ================================================
// 0xcb9e: WORD 'PFIL-VOC' codep=0x1ab5 wordp=0xcba7
// ================================================
// 0xcba7: db 0xb5 0xcb 0x74 0xcb 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '  t                 '

// ================================================
// 0xcbbb: WORD 'PFIL-OV' codep=0x83dd wordp=0xcbc4
// ================================================
// Overlay PFIL_dash_OV = 0x7418

// ================================================
// 0xcbc6: WORD 'U-PF' codep=0x224c wordp=0xcbcf
// ================================================

void U_dash_PF() // U-PF
{
  LoadOverlay(PFIL_dash_OV); // PFIL-OV
  _ro_U_dash_PFILE_rc_(); // Overlay PERSONNEL-OV
}


// ================================================
// 0xcbd5: WORD 'SHIP-GR-VOC' codep=0x1ab5 wordp=0xcbe1
// ================================================
// 0xcbe1: db 0xef 0xcb 0xa9 0xcb 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '                    '

// ================================================
// 0xcbf5: WORD 'SHIP-GR-OV' codep=0x83dd wordp=0xcc01
// ================================================
// Overlay SHIP_dash_GR_dash_OV = 0x7531

// ================================================
// 0xcc03: WORD 'OV.MASS' codep=0x224c wordp=0xcc0f
// ================================================

void OV_dot_MASS() // OV.MASS
{
  LoadOverlay(SHIP_dash_GR_dash_OV); // SHIP-GR-OV
  _ro__dot_MASS_rc_(); // Overlay SHIPGRPH-OV
}


// ================================================
// 0xcc15: WORD 'OV.ACC' codep=0x224c wordp=0xcc20
// ================================================

void OV_dot_ACC() // OV.ACC
{
  LoadOverlay(SHIP_dash_GR_dash_OV); // SHIP-GR-OV
  _ro__dot_ACC_rc_(); // Overlay SHIPGRPH-OV
}


// ================================================
// 0xcc26: WORD 'OV.PODS' codep=0x224c wordp=0xcc32
// ================================================

void OV_dot_PODS() // OV.PODS
{
  LoadOverlay(SHIP_dash_GR_dash_OV); // SHIP-GR-OV
  _ro__dot_PODS_rc_(); // Overlay SHIPGRPH-OV
}


// ================================================
// 0xcc38: WORD 'OV.SHIP' codep=0x224c wordp=0xcc44
// ================================================

void OV_dot_SHIP() // OV.SHIP
{
  LoadOverlay(SHIP_dash_GR_dash_OV); // SHIP-GR-OV
  _ro__dot_SHIP_rc_(); // Overlay SHIPGRPH-OV
}


// ================================================
// 0xcc4a: WORD 'OVBALAN' codep=0x224c wordp=0xcc56
// ================================================

void OVBALAN() // OVBALAN
{
  LoadOverlay(SHIP_dash_GR_dash_OV); // SHIP-GR-OV
  _ro_BALANC(); // Overlay SHIPGRPH-OV
}


// ================================================
// 0xcc5c: WORD 'OV.CONF' codep=0x224c wordp=0xcc68
// ================================================

void OV_dot_CONF() // OV.CONF
{
  LoadOverlay(SHIP_dash_GR_dash_OV); // SHIP-GR-OV
  _ro__dot_CONFI(); // Overlay SHIPGRPH-OV
}


// ================================================
// 0xcc6e: WORD 'CONFIG-' codep=0x1ab5 wordp=0xcc7a
// ================================================
// 0xcc7a: db 0x88 0xcc 0xe3 0xcb 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '                    '

// ================================================
// 0xcc8e: WORD 'CONFIG-OV' codep=0x83dd wordp=0xcc9c
// ================================================
// Overlay CONFIG_dash_OV = 0x75bc

// ================================================
// 0xcc9e: WORD 'U-SC' codep=0x224c wordp=0xcca7
// ================================================

void U_dash_SC() // U-SC
{
  LoadOverlay(CONFIG_dash_OV); // CONFIG-OV
  _ro_U_dash_CONFIG_rc_(); // Overlay CONFIG-OV
}


// ================================================
// 0xccad: WORD 'TD-VOC' codep=0x1ab5 wordp=0xccb8
// ================================================
// 0xccb8: db 0xc6 0xcc 0x7c 0xcc 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '  |                 '

// ================================================
// 0xcccc: WORD 'TD-OV' codep=0x83dd wordp=0xccd6
// ================================================
// Overlay TD_dash_OV = 0x76a4

// ================================================
// 0xccd8: WORD 'U-TD' codep=0x224c wordp=0xcce1
// ================================================

void U_dash_TD() // U-TD
{
  LoadOverlay(TD_dash_OV); // TD-OV
  _ro_U_dash_DEPOT_rc_(); // Overlay TDEPOT-OV
}


// ================================================
// 0xcce7: WORD 'TOSS' codep=0x224c wordp=0xccf0
// ================================================

void TOSS() // TOSS
{
  LoadOverlay(TD_dash_OV); // TD-OV
  SELL_dash_IT(); // Overlay TDEPOT-OV
}


// ================================================
// 0xccf6: WORD 'KEY-EL' codep=0x224c wordp=0xcd01
// ================================================

void KEY_dash_EL() // KEY-EL
{
  LoadOverlay(TD_dash_OV); // TD-OV
  KEY_dash_ELEM_dash_AMT(); // Overlay TDEPOT-OV
}


// ================================================
// 0xcd07: WORD 'OP-VOC' codep=0x1ab5 wordp=0xcd12
// ================================================
// 0xcd12: db 0x20 0xcd 0xba 0xcc 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '                    '

// ================================================
// 0xcd26: WORD 'OP-OV' codep=0x83dd wordp=0xcd30
// ================================================
// Overlay OP_dash_OV = 0x88f9

// ================================================
// 0xcd32: WORD 'U-OP' codep=0x224c wordp=0xcd3b
// ================================================

void U_dash_OP() // U-OP
{
  LoadOverlay(OP_dash_OV); // OP-OV
  _ro_U_dash_OP_rc_(); // Overlay OP-OV
}


// ================================================
// 0xcd41: WORD 'VITA' codep=0x1ab5 wordp=0xcd4a
// ================================================
// 0xcd4a: db 0x58 0xcd 0x14 0xcd 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 'X                   '

// ================================================
// 0xcd5e: WORD 'VITA-OV' codep=0x83dd wordp=0xcd6a
// ================================================
// Overlay VITA_dash_OV = 0x7825

// ================================================
// 0xcd6c: WORD 'DIO' codep=0x224c wordp=0xcd74
// ================================================

void DIO() // DIO
{
  LoadOverlay(VITA_dash_OV); // VITA-OV
  DIO(); // Overlay VITA-OV
}


// ================================================
// 0xcd7a: WORD 'MAPS-VOC' codep=0x1ab5 wordp=0xcd87
// ================================================
// 0xcd87: db 0x95 0xcd 0x4c 0xcd 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '  L                 '

// ================================================
// 0xcd9b: WORD 'MAPS-' codep=0x83dd wordp=0xcda5
// ================================================
// Overlay MAPS_dash_ = 0x8d5c

// ================================================
// 0xcda7: WORD 'MAPS' codep=0x224c wordp=0xcdb0
// ================================================

void MAPS() // MAPS
{
  LoadOverlay(MAPS_dash_); // MAPS-
  MAPS(); // Overlay MAPS-OV
}


// ================================================
// 0xcdb6: WORD '.STOR' codep=0x224c wordp=0xcdc0
// ================================================

void DrawSTOR() // .STOR
{
  LoadOverlay(MAPS_dash_); // MAPS-
  DrawSTORM(); // Overlay MAPS-OV
}


// ================================================
// 0xcdc6: WORD '(2X2)' codep=0x224c wordp=0xcdd0
// ================================================

void _ro_2X2_rc_() // (2X2)
{
  LoadOverlay(MAPS_dash_); // MAPS-
  _2X2CONTOUR(); // Overlay MAPS-OV
}


// ================================================
// 0xcdd6: WORD '2X2CO' codep=0x224c wordp=0xcde0 params=0 returns=0
// ================================================

void _2X2CO() // 2X2CO
{
  Push(0xcdd0); // '(2X2)'
  MODULE(); // MODULE
}


// ================================================
// 0xcde8: WORD '(4X4)' codep=0x224c wordp=0xcdf2
// ================================================

void _ro_4X4_rc_() // (4X4)
{
  LoadOverlay(MAPS_dash_); // MAPS-
  _4X4CONTOUR(); // Overlay MAPS-OV
}


// ================================================
// 0xcdf8: WORD '4X4CO' codep=0x224c wordp=0xce02 params=0 returns=0
// ================================================

void _4X4CO() // 4X4CO
{
  Push(0xcdf2); // '(4X4)'
  MODULE(); // MODULE
}


// ================================================
// 0xce0a: WORD '(8X8)' codep=0x224c wordp=0xce14
// ================================================

void _ro_8X8_rc_() // (8X8)
{
  LoadOverlay(MAPS_dash_); // MAPS-
  _8X8CONTOUR(); // Overlay MAPS-OV
}


// ================================================
// 0xce1a: WORD '8X8CO' codep=0x224c wordp=0xce24 params=0 returns=0
// ================================================

void _8X8CO() // 8X8CO
{
  Push(0xce14); // '(8X8)'
  MODULE(); // MODULE
}


// ================================================
// 0xce2c: WORD 'STO-VOC' codep=0x1ab5 wordp=0xce34
// ================================================
// 0xce34: db 0x42 0xce 0x89 0xcd 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 'B                   '

// ================================================
// 0xce48: WORD 'STO-OV' codep=0x83dd wordp=0xce50
// ================================================
// Overlay STO_dash_OV = 0x8de4

// ================================================
// 0xce52: WORD 'DO.' codep=0x224c wordp=0xce5a
// ================================================

void DO_dot_() // DO.
{
  LoadOverlay(STO_dash_OV); // STO-OV
  _ro_DO_dot_STORM(); // Overlay STORM-OV
}


// ================================================
// 0xce60: WORD 'INJ' codep=0x224c wordp=0xce68
// ================================================

void INJ() // INJ
{
  LoadOverlay(STO_dash_OV); // STO-OV
  INJURE_dash_PL(); // Overlay STORM-OV
}


// ================================================
// 0xce6e: WORD 'DRONE-V' codep=0x1ab5 wordp=0xce7a
// ================================================
// 0xce7a: db 0x88 0xce 0x36 0xce 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '  6                 '

// ================================================
// 0xce8e: WORD 'DRONE-O' codep=0x83dd wordp=0xce9a
// ================================================
// Overlay DRONE_dash_O = 0x95d0

// ================================================
// 0xce9c: WORD 'DRN' codep=0x224c wordp=0xcea4
// ================================================

void DRN() // DRN
{
  LoadOverlay(DRONE_dash_O); // DRONE-O
  DRONE(); // Overlay DRONE
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xceac: WORD 'RECAL' codep=0x224c wordp=0xceb6
// ================================================

void RECAL() // RECAL
{
  LoadOverlay(DRONE_dash_O); // DRONE-O
  RECALL(); // Overlay DRONE
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xcebe: WORD 'DRONE' codep=0x224c wordp=0xcec8 params=0 returns=0
// ================================================

void DRONE() // DRONE
{
  Push(0xcea4); // 'DRN'
  MODULE(); // MODULE
}


// ================================================
// 0xced0: WORD 'TVCON-V' codep=0x1ab5 wordp=0xcedc
// ================================================
// 0xcedc: db 0xea 0xce 0x7c 0xce 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '  |                 '

// ================================================
// 0xcef0: WORD 'TVCON-OV' codep=0x83dd wordp=0xcefd
// ================================================
// Overlay TVCON_dash_OV = 0x9491

// ================================================
// 0xceff: WORD 'DOST' codep=0x224c wordp=0xcf08
// ================================================

void DOST() // DOST
{
  LoadOverlay(TVCON_dash_OV); // TVCON-OV
  DO_dash_STORM(); // Overlay TVCON-OV
}


// ================================================
// 0xcf0e: WORD 'DO-STORM' codep=0x224c wordp=0xcf1b params=0 returns=0
// ================================================

void DO_dash_STORM() // DO-STORM
{
  Push(0xcf08); // 'DOST'
  MODULE(); // MODULE
}


// ================================================
// 0xcf23: WORD '.ST' codep=0x224c wordp=0xcf2b
// ================================================

void DrawST() // .ST
{
  LoadOverlay(TVCON_dash_OV); // TVCON-OV
  DrawSTATS(); // Overlay TVCON-OV
}


// ================================================
// 0xcf31: WORD '.WH' codep=0x224c wordp=0xcf39
// ================================================

void DrawWH() // .WH
{
  LoadOverlay(TVCON_dash_OV); // TVCON-OV
  DrawWHERE(); // Overlay TVCON-OV
}


// ================================================
// 0xcf3f: WORD '.DA' codep=0x224c wordp=0xcf47
// ================================================

void DrawDA() // .DA
{
  LoadOverlay(TVCON_dash_OV); // TVCON-OV
  DrawSTARDATE(); // Overlay TVCON-OV
}


// ================================================
// 0xcf4d: WORD '.EN' codep=0x224c wordp=0xcf55
// ================================================

void DrawEN() // .EN
{
  LoadOverlay(TVCON_dash_OV); // TVCON-OV
  DrawENERGY(); // Overlay TVCON-OV
}


// ================================================
// 0xcf5b: WORD '.CA' codep=0x224c wordp=0xcf63
// ================================================

void DrawCA() // .CA
{
  LoadOverlay(TVCON_dash_OV); // TVCON-OV
  DrawCARGO(); // Overlay TVCON-OV
}


// ================================================
// 0xcf69: WORD '.DI' codep=0x224c wordp=0xcf71
// ================================================

void DrawDI() // .DI
{
  LoadOverlay(TVCON_dash_OV); // TVCON-OV
  DrawDISTANCE(); // Overlay TVCON-OV
}


// ================================================
// 0xcf77: WORD 'SEED-VOC' codep=0x1ab5 wordp=0xcf80
// ================================================
// 0xcf80: db 0x8e 0xcf 0xde 0xce 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '                    '

// ================================================
// 0xcf94: WORD 'SEED-OV' codep=0x83dd wordp=0xcf9d
// ================================================
// Overlay SEED_dash_OV = 0x7ddd

// ================================================
// 0xcf9f: WORD 'POPU' codep=0x224c wordp=0xcfa8
// ================================================

void POPU() // POPU
{
  LoadOverlay(SEED_dash_OV); // SEED-OV
  POPULA(); // Overlay SEED-OV
}


// ================================================
// 0xcfae: WORD 'ITEMS' codep=0x1ab5 wordp=0xcfb8
// ================================================
// 0xcfb8: db 0xc6 0xcf 0x82 0xcf 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '                    '

// ================================================
// 0xcfcc: WORD 'ITEMS-OV' codep=0x83dd wordp=0xcfd9
// ================================================
// Overlay ITEMS_dash_OV = 0x89f7

// ================================================
// 0xcfdb: WORD '/ITEMS' codep=0x224c wordp=0xcfe6
// ================================================

void _slash_ITEMS() // /ITEMS
{
  LoadOverlay(ITEMS_dash_OV); // ITEMS-OV
  _ro__slash_ITEMS_rc_(); // Overlay TVITEMS
}


// ================================================
// 0xcfec: WORD '>DEBRIS' codep=0x224c wordp=0xcff8
// ================================================

void _gt_DEBRIS() // >DEBRIS
{
  LoadOverlay(ITEMS_dash_OV); // ITEMS-OV
  _gt_DEBRIS(); // Overlay TVITEMS
}


// ================================================
// 0xcffe: WORD 'ICON-V' codep=0x1ab5 wordp=0xd009
// ================================================
// 0xd009: db 0x17 0xd0 0xba 0xcf 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '                    '

// ================================================
// 0xd01d: WORD 'LISTIC' codep=0x83dd wordp=0xd028
// ================================================
// Overlay LISTIC = 0x7e72

// ================================================
// 0xd02a: WORD 'ICONS' codep=0x224c wordp=0xd034
// ================================================

void ICONS() // ICONS
{
  LoadOverlay(LISTIC); // LISTIC
  ICONS(); // Overlay LISTICONS
}


// ================================================
// 0xd03a: WORD 'MOVE-VOC' codep=0x1ab5 wordp=0xd047
// ================================================
// 0xd047: db 0x55 0xd0 0x0b 0xd0 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 'U                   '

// ================================================
// 0xd05b: WORD 'MOVE-OV' codep=0x83dd wordp=0xd067
// ================================================
// Overlay MOVE_dash_OV = 0x7e9d

// ================================================
// 0xd069: WORD 'TVMO' codep=0x224c wordp=0xd072
// ================================================

void TVMO() // TVMO
{
  LoadOverlay(MOVE_dash_OV); // MOVE-OV
  TV_dash_MOVE(); // Overlay MOVE-OV
}


// ================================================
// 0xd078: WORD '?POP' codep=0x224c wordp=0xd081
// ================================================

void IsPOP() // ?POP
{
  LoadOverlay(MOVE_dash_OV); // MOVE-OV
  IsPOPULA(); // Overlay MOVE-OV
}


// ================================================
// 0xd087: WORD '!E/M' codep=0x224c wordp=0xd090
// ================================================

void StoreE_slash_M() // !E/M
{
  LoadOverlay(MOVE_dash_OV); // MOVE-OV
  SET_dash_SPE(); // Overlay MOVE-OV
}


// ================================================
// 0xd096: WORD 'TELE_1' codep=0x224c wordp=0xd09f
// ================================================

void TELE_1() // TELE_1
{
  LoadOverlay(MOVE_dash_OV); // MOVE-OV
  TELE(); // Overlay MOVE-OV
}


// ================================================
// 0xd0a5: WORD 'TELE_2' codep=0x224c wordp=0xd0ae params=0 returns=0
// ================================================

void TELE_2() // TELE_2
{
  Push(0xd09f); // 'TELE_1'
  MODULE(); // MODULE
}


// ================================================
// 0xd0b6: WORD 'T.V.' codep=0x1ab5 wordp=0xd0bf
// ================================================
// 0xd0bf: db 0xcd 0xd0 0x49 0xd0 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '  I                 '

// ================================================
// 0xd0d3: WORD 'TV-OV' codep=0x83dd wordp=0xd0dd
// ================================================
// Overlay TV_dash_OV = 0x7a1a

// ================================================
// 0xd0df: WORD 'TV-DI' codep=0x224c wordp=0xd0e9
// ================================================

void TV_dash_DI() // TV-DI
{
  LoadOverlay(TV_dash_OV); // TV-OV
  TV_dash_DISP(); // Overlay TV-OV
}


// ================================================
// 0xd0ef: WORD '(TVSE' codep=0x224c wordp=0xd0f9
// ================================================

void _ro_TVSE() // (TVSE
{
  LoadOverlay(TV_dash_OV); // TV-OV
  TV_dash_SETUP(); // Overlay TV-OV
}


// ================================================
// 0xd0ff: WORD '(CLEA' codep=0x224c wordp=0xd109
// ================================================

void _ro_CLEA() // (CLEA
{
  LoadOverlay(TV_dash_OV); // TV-OV
  TV_dash_CLEANU(); // Overlay TV-OV
}


// ================================================
// 0xd10f: WORD '(HEAV' codep=0x224c wordp=0xd119
// ================================================

void _ro_HEAV() // (HEAV
{
  LoadOverlay(TV_dash_OV); // TV-OV
  HEAVEHO(); // Overlay TV-OV
}


// ================================================
// 0xd11f: WORD '(WEAP' codep=0x224c wordp=0xd129
// ================================================

void _ro_WEAP() // (WEAP
{
  LoadOverlay(TV_dash_OV); // TV-OV
  DO_dot_WEAPON(); // Overlay TV-OV
}


// ================================================
// 0xd12f: WORD '(TALK' codep=0x224c wordp=0xd139
// ================================================

void _ro_TALK() // (TALK
{
  LoadOverlay(TV_dash_OV); // TV-OV
  WALK_and_TALK(); // Overlay TV-OV
}


// ================================================
// 0xd13f: WORD 'HEAVE' codep=0x224c wordp=0xd149 params=0 returns=0
// ================================================

void HEAVE() // HEAVE
{
  SAVE_dash_OV(); // SAVE-OV
  Push(0xd119); // '(HEAV'
  MODULE(); // MODULE
}


// ================================================
// 0xd153: WORD 'BEHAVIO' codep=0x1ab5 wordp=0xd15f
// ================================================
// 0xd15f: db 0x6d 0xd1 0xc1 0xd0 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 'm                   '

// ================================================
// 0xd173: WORD 'BEHAV' codep=0x83dd wordp=0xd17d
// ================================================
// Overlay BEHAV = 0x8aeb

// ================================================
// 0xd17f: WORD 'TVTAS' codep=0x224c wordp=0xd189
// ================================================

void TVTAS() // TVTAS
{
  LoadOverlay(BEHAV); // BEHAV
  TV_dash_TASKS(); // Overlay BEHAV-OV
}


// ================================================
// 0xd18f: WORD 'TV' codep=0x224c wordp=0xd196
// ================================================

void TV() // TV
{
  LoadOverlay(BEHAV); // BEHAV
  TV(); // Overlay BEHAV-OV
}


// ================================================
// 0xd19c: WORD '(SIMU' codep=0x224c wordp=0xd1a6
// ================================================

void _ro_SIMU() // (SIMU
{
  LoadOverlay(BEHAV); // BEHAV
  _1SIMULATE(); // Overlay BEHAV-OV
}


// ================================================
// 0xd1ac: WORD 'SIMUL' codep=0x224c wordp=0xd1b6 params=0 returns=0
// ================================================

void SIMUL() // SIMUL
{
  Push(0xd1a6); // '(SIMU'
  MODULE(); // MODULE
}


// ================================================
// 0xd1be: WORD 'FSTN' codep=0x224c wordp=0xd1c7
// ================================================

void FSTN() // FSTN
{
  LoadOverlay(BEHAV); // BEHAV
  CIRCLES(); // Overlay BEHAV-OV
}


// ================================================
// 0xd1cd: WORD 'DSTUN' codep=0x224c wordp=0xd1d7 params=0 returns=0
// ================================================

void DSTUN() // DSTUN
{
  Push(0xd1c7); // 'FSTN'
  MODULE(); // MODULE
}


// ================================================
// 0xd1df: WORD 'STP-VOC' codep=0x1ab5 wordp=0xd1eb
// ================================================
// 0xd1eb: db 0xf9 0xd1 0x61 0xd1 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '  a                 '

// ================================================
// 0xd1ff: WORD 'STP-OV' codep=0x83dd wordp=0xd20a
// ================================================
// Overlay STP_dash_OV = 0x90db

// ================================================
// 0xd20c: WORD 'SECUR' codep=0x224c wordp=0xd216
// ================================================

void SECUR() // SECUR
{
  LoadOverlay(STP_dash_OV); // STP-OV
  SP(); // Overlay STP-OV
}


// ================================================
// 0xd21c: WORD 'SIC'E' codep=0x224c wordp=0xd226
// ================================================

void SIC_i_E() // SIC'E
{
  LoadOverlay(STP_dash_OV); // STP-OV
  MP_i_ER(); // Overlay STP-OV
}


// ================================================
// 0xd22c: WORD '2NDS' codep=0x224c wordp=0xd235
// ================================================

void _2NDS() // 2NDS
{
  LoadOverlay(STP_dash_OV); // STP-OV
  _2N(); // Overlay STP-OV
}


// ================================================
// 0xd23b: WORD 'LAUNCH-V' codep=0x1ab5 wordp=0xd248
// ================================================
// 0xd248: db 0x56 0xd2 0xed 0xd1 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 'V                   '

// ================================================
// 0xd25c: WORD 'LAUNC' codep=0x83dd wordp=0xd266
// ================================================
// Overlay LAUNC = 0x8889

// ================================================
// 0xd268: WORD 'OVCOU' codep=0x224c wordp=0xd272
// ================================================

void OVCOU() // OVCOU
{
  LoadOverlay(LAUNC); // LAUNC
  _and_LAUNCH(); // Overlay LAUNCH-OV
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xd27a: WORD 'OVBAC' codep=0x224c wordp=0xd284
// ================================================

void OVBAC() // OVBAC
{
  LoadOverlay(LAUNC); // LAUNC
  _and_RETURN(); // Overlay LAUNCH-OV
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xd28c: WORD '.AIRL' codep=0x224c wordp=0xd296
// ================================================

void DrawAIRL() // .AIRL
{
  LoadOverlay(LAUNC); // LAUNC
  DrawAIRLOCK(); // Overlay LAUNCH-OV
}


// ================================================
// 0xd29c: WORD 'CAP-VOC' codep=0x1ab5 wordp=0xd2a8
// ================================================
// 0xd2a8: db 0xb6 0xd2 0x4a 0xd2 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '  J                 '

// ================================================
// 0xd2bc: WORD 'CAP-O' codep=0x83dd wordp=0xd2c6
// ================================================
// Overlay CAP_dash_O = 0x81d1

// ================================================
// 0xd2c8: WORD 'OV/IT' codep=0x224c wordp=0xd2d2
// ================================================

void OV_slash_IT() // OV/IT
{
  LoadOverlay(CAP_dash_O); // CAP-O
  _ro__slash_ITEMS_rc_(); // Overlay CAPTAIN
}


// ================================================
// 0xd2d8: WORD 'OV/LA' codep=0x224c wordp=0xd2e2
// ================================================

void OV_slash_LA() // OV/LA
{
  LoadOverlay(CAP_dash_O); // CAP-O
  _ro__slash_LAUNCH_slash_LAND_rc_(); // Overlay CAPTAIN
}


// ================================================
// 0xd2e8: WORD 'OV>DE' codep=0x224c wordp=0xd2f2
// ================================================

void OV_gt_DE() // OV>DE
{
  LoadOverlay(CAP_dash_O); // CAP-O
  _gt_DESCEND(); // Overlay CAPTAIN
}


// ================================================
// 0xd2f8: WORD 'OV/LO' codep=0x224c wordp=0xd302
// ================================================

void OV_slash_LO() // OV/LO
{
  LoadOverlay(CAP_dash_O); // CAP-O
  _ro__slash_LOG_dash_PLAN_rc_(); // Overlay CAPTAIN
}


// ================================================
// 0xd308: WORD 'DOC-VOC' codep=0x1ab5 wordp=0xd314
// ================================================
// 0xd314: db 0x22 0xd3 0xaa 0xd2 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '"                   '

// ================================================
// 0xd328: WORD 'DOC-OV' codep=0x83dd wordp=0xd333
// ================================================
// Overlay DOC_dash_OV = 0x7fdd

// ================================================
// 0xd335: WORD 'OV/EX' codep=0x224c wordp=0xd33f
// ================================================

void OV_slash_EX() // OV/EX
{
  LoadOverlay(DOC_dash_OV); // DOC-OV
  _ro__slash_EXAMINE_rc_(); // Overlay DOCTOR
}


// ================================================
// 0xd345: WORD 'OV/TR' codep=0x224c wordp=0xd34f
// ================================================

void OV_slash_TR() // OV/TR
{
  LoadOverlay(DOC_dash_OV); // DOC-OV
  _ro__slash_TREAT_rc_(); // Overlay DOCTOR
}


// ================================================
// 0xd355: WORD 'AN-VOC' codep=0x1ab5 wordp=0xd360
// ================================================
// 0xd360: db 0x6e 0xd3 0x16 0xd3 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 'n                   '

// ================================================
// 0xd374: WORD 'AN-OV' codep=0x83dd wordp=0xd37e
// ================================================
// Overlay AN_dash_OV = 0x87f8

// ================================================
// 0xd380: WORD 'OV/AN' codep=0x224c wordp=0xd38a
// ================================================

void OV_slash_AN() // OV/AN
{
  LoadOverlay(AN_dash_OV); // AN-OV
  _ro__slash_ANALYSIS_rc_(); // Overlay ANALYZE-OV
}


// ================================================
// 0xd390: WORD '*MAP' codep=0x1ab5 wordp=0xd399
// ================================================
// 0xd399: db 0xa7 0xd3 0x62 0xd3 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '  b                 '

// ================================================
// 0xd3ad: WORD '*MAP-OV' codep=0x83dd wordp=0xd3b9
// ================================================
// Overlay _star_MAP_dash_OV = 0x8518

// ================================================
// 0xd3bb: WORD 'OV/ST' codep=0x224c wordp=0xd3c5
// ================================================

void OV_slash_ST() // OV/ST
{
  LoadOverlay(_star_MAP_dash_OV); // *MAP-OV
  _ro__slash_STARMAP_rc_(); // Overlay MAP-OV
  Push(pp_FTRIG); // FTRIG
  _099(); // 099
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xd3d1: WORD 'JUMP-VO' codep=0x1ab5 wordp=0xd3dd
// ================================================
// 0xd3dd: db 0xeb 0xd3 0x9b 0xd3 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '                    '

// ================================================
// 0xd3f1: WORD 'JUMP-OV' codep=0x83dd wordp=0xd3fd
// ================================================
// Overlay JUMP_dash_OV = 0x952d

// ================================================
// 0xd3ff: WORD 'JUMP' codep=0x224c wordp=0xd408
// ================================================

void JUMP() // JUMP
{
  LoadOverlay(JUMP_dash_OV); // JUMP-OV
  JUMP(); // Overlay JUMP
}


// ================================================
// 0xd40e: WORD 'LAND-VO' codep=0x1ab5 wordp=0xd41a
// ================================================
// 0xd41a: db 0x28 0xd4 0xdf 0xd3 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '(                   '

// ================================================
// 0xd42e: WORD 'LAND-' codep=0x83dd wordp=0xd438
// ================================================
// Overlay LAND_dash_ = 0x9099

// ================================================
// 0xd43a: WORD 'TRY-L_2' codep=0x224c wordp=0xd444
// ================================================

void TRY_dash_L_2() // TRY-L_2
{
  LoadOverlay(LAND_dash_); // LAND-
  IsLAND(); // Overlay LAND-OV
}


// ================================================
// 0xd44a: WORD 'TRY-L_1' codep=0x224c wordp=0xd454
// ================================================

void TRY_dash_L_1() // TRY-L_1
{
  LoadOverlay(LAND_dash_); // LAND-
  IsLAUNCH(); // Overlay LAND-OV
}


// ================================================
// 0xd45a: WORD 'BTN-VOC' codep=0x1ab5 wordp=0xd466
// ================================================
// 0xd466: db 0x74 0xd4 0x1c 0xd4 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 't                   '

// ================================================
// 0xd47a: WORD 'BTN-OV' codep=0x83dd wordp=0xd485
// ================================================
// Overlay BTN_dash_OV = 0x8480

// ================================================
// 0xd487: WORD 'OVFLT' codep=0x224c wordp=0xd491
// ================================================

void OVFLT() // OVFLT
{
  LoadOverlay(BTN_dash_OV); // BTN-OV
  _gt_FLT(); // Overlay SHIPBUTTONS
}


// ================================================
// 0xd497: WORD 'CHKFLT' codep=0x1ab5 wordp=0xd4a2
// ================================================
// 0xd4a2: db 0xb0 0xd4 0x68 0xd4 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '  h                 '

// ================================================
// 0xd4b6: WORD 'CHKFL' codep=0x83dd wordp=0xd4c0
// ================================================
// Overlay CHKFL = 0x6f65

// ================================================
// 0xd4c2: WORD '?FLIG' codep=0x224c wordp=0xd4cc
// ================================================

void IsFLIG() // ?FLIG
{
  LoadOverlay(CHKFL); // CHKFL
  IsCAN_dash_LEAVE(); // Overlay CHKFLIGHT-OV
}


// ================================================
// 0xd4d2: WORD 'PM-VOC' codep=0x1ab5 wordp=0xd4dd
// ================================================
// 0xd4dd: db 0xeb 0xd4 0xa4 0xd4 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '                    '

// ================================================
// 0xd4f1: WORD 'PM-OV' codep=0x83dd wordp=0xd4fb
// ================================================
// Overlay PM_dash_OV = 0x779d

// ================================================
// 0xd4fd: WORD 'PORTM' codep=0x224c wordp=0xd507
// ================================================

void PORTM() // PORTM
{
  LoadOverlay(PM_dash_OV); // PM-OV
  WALKIES(); // Overlay PORTMENU-OV
}


// ================================================
// 0xd50d: WORD 'PPIC' codep=0x224c wordp=0xd516
// ================================================

void PPIC() // PPIC
{
  LoadOverlay(PM_dash_OV); // PM-OV
  PPIC(); // Overlay PORTMENU-OV
}


// ================================================
// 0xd51c: WORD 'GAME' codep=0x1ab5 wordp=0xd525
// ================================================
// 0xd525: db 0x33 0xd5 0xdf 0xd4 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xff 0xff 0x1a 0x08 '3                   '

// ================================================
// 0xd539: WORD 'GAME-OV' codep=0x83dd wordp=0xd545
// ================================================
// Overlay GAME_dash_OV = 0x3c64

// ================================================
// 0xd547: WORD 'STARTGA' codep=0x224c wordp=0xd553
// ================================================

void STARTGA() // STARTGA
{
  LoadOverlay(GAME_dash_OV); // GAME-OV
  START(); // Overlay MODGAME
}


// ================================================
// 0xd559: WORD 'ERR!' codep=0x224c wordp=0xd562
// ================================================

void ERR_ex_() // ERR!
{
  LoadOverlay(GAME_dash_OV); // GAME-OV
  ERR_ex_(); // Overlay MODGAME
}


// ================================================
// 0xd568: WORD 'MERR' codep=0x224c wordp=0xd571 params=0 returns=0
// ================================================

void MERR() // MERR
{
  Push(0xd562); // 'ERR!'
  MODULE(); // MODULE
}


// ================================================
// 0xd579: WORD 'GAMEOPS' codep=0x224c wordp=0xd585
// ================================================

void GAMEOPS() // GAMEOPS
{
  LoadOverlay(GAME_dash_OV); // GAME-OV
  GAMEO_1(); // Overlay MODGAME
  SAVE_dash_OV(); // SAVE-OV
}


// ================================================
// 0xd58d: WORD 'GAMEOPM' codep=0x224c wordp=0xd599 params=0 returns=0
// ================================================

void GAMEOPM() // GAMEOPM
{
  Push(0xd585); // 'GAMEOPS'
  MODULE(); // MODULE
}


// ================================================
// 0xd5a1: WORD 'BOSS' codep=0x224c wordp=0xd5aa
// ================================================

void BOSS() // BOSS
{
  LoadOverlay(GAME_dash_OV); // GAME-OV
  BOSS(); // Overlay MODGAME
}


// ================================================
// 0xd5b0: WORD 'DBS' codep=0x224c wordp=0xd5b8 params=0 returns=0
// ================================================

void DBS() // DBS
{
  Push(0xd5aa); // 'BOSS'
  MODULE(); // MODULE
}


// ================================================
// 0xd5c0: WORD 'LET-THE' codep=0x224c wordp=0xd5cc
// ================================================

void LET_dash_THE() // LET-THE
{
  Push(pp_ESC_dash_EN); // ESC-EN
  _099(); // 099
  MOUNTA(); // MOUNTA
  Push(0xb5f1); // 'TALLOC'
  MODULE(); // MODULE
  STARTER(); // STARTER
  Push(pp_RELAXTI); // RELAXTI
  ON_2(); // ON_2
  MOUNTA(); // MOUNTA
  SIGFLD(":TIMEST");
  Push(Read16(Pop())); //  @
  Push(pp_TIMESTA); // TIMESTA
  Store_2(); // !_2
  Push(pp_RELAXTI); // RELAXTI
  _099(); // 099
  STARTGA(); // STARTGA
}

// 0xd5f0: db 0x00 ' '

