/*
 * book.c -- code for probing Polyglot opening books
 *
 * This code was first released in the public domain by Michel Van den Bergh.
 * The array Random64 is taken from the Polyglot source code.
 * I am pretty sure that a table of random numbers is never protected
 * by copyright.
 *
 * It s adapted by H.G. Muller for working with xboard / Winboard
 *
 * The following terms apply to the enhanced version of XBoard distributed
 * by the Free Software Foundation:
 * ------------------------------------------------------------------------
 *
 * GNU XBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * GNU XBoard is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.  *
 *
 * ------------------------------------------------------------------------
 */

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

#include "common.h"
#include "frontend.h"
#include "backend.h"
#include "moves.h"
#include "gettext.h"

#ifdef ENABLE_NLS
# define  _(s) gettext (s)
# define N_(s) gettext_noop (s)
#else
# define  _(s) (s)
# define N_(s)  s
#endif

#ifdef _MSC_VER
  typedef unsigned __int64 uint64;
#else
  typedef unsigned long long int uint64;
#endif

#ifdef _MSC_VER
#  define U64(u) (u##ui64)
#else
#  define U64(u) (u##ULL)
#endif

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

typedef struct {
    uint64 key;
    uint16 move;
    uint16 weight;
    uint16 learnPoints;
    uint16 learnCount;
} entry_t;

entry_t entry_none = {
    0, 0, 0, 0
};

char *promote_pieces=" nbrqac=+";

uint64 Random64[781] = {
   U64(0x9D39247E33776D41), U64(0x2AF7398005AAA5C7), U64(0x44DB015024623547), U64(0x9C15F73E62A76AE2),
   U64(0x75834465489C0C89), U64(0x3290AC3A203001BF), U64(0x0FBBAD1F61042279), U64(0xE83A908FF2FB60CA),
   U64(0x0D7E765D58755C10), U64(0x1A083822CEAFE02D), U64(0x9605D5F0E25EC3B0), U64(0xD021FF5CD13A2ED5),
   U64(0x40BDF15D4A672E32), U64(0x011355146FD56395), U64(0x5DB4832046F3D9E5), U64(0x239F8B2D7FF719CC),
   U64(0x05D1A1AE85B49AA1), U64(0x679F848F6E8FC971), U64(0x7449BBFF801FED0B), U64(0x7D11CDB1C3B7ADF0),
   U64(0x82C7709E781EB7CC), U64(0xF3218F1C9510786C), U64(0x331478F3AF51BBE6), U64(0x4BB38DE5E7219443),
   U64(0xAA649C6EBCFD50FC), U64(0x8DBD98A352AFD40B), U64(0x87D2074B81D79217), U64(0x19F3C751D3E92AE1),
   U64(0xB4AB30F062B19ABF), U64(0x7B0500AC42047AC4), U64(0xC9452CA81A09D85D), U64(0x24AA6C514DA27500),
   U64(0x4C9F34427501B447), U64(0x14A68FD73C910841), U64(0xA71B9B83461CBD93), U64(0x03488B95B0F1850F),
   U64(0x637B2B34FF93C040), U64(0x09D1BC9A3DD90A94), U64(0x3575668334A1DD3B), U64(0x735E2B97A4C45A23),
   U64(0x18727070F1BD400B), U64(0x1FCBACD259BF02E7), U64(0xD310A7C2CE9B6555), U64(0xBF983FE0FE5D8244),
   U64(0x9F74D14F7454A824), U64(0x51EBDC4AB9BA3035), U64(0x5C82C505DB9AB0FA), U64(0xFCF7FE8A3430B241),
   U64(0x3253A729B9BA3DDE), U64(0x8C74C368081B3075), U64(0xB9BC6C87167C33E7), U64(0x7EF48F2B83024E20),
   U64(0x11D505D4C351BD7F), U64(0x6568FCA92C76A243), U64(0x4DE0B0F40F32A7B8), U64(0x96D693460CC37E5D),
   U64(0x42E240CB63689F2F), U64(0x6D2BDCDAE2919661), U64(0x42880B0236E4D951), U64(0x5F0F4A5898171BB6),
   U64(0x39F890F579F92F88), U64(0x93C5B5F47356388B), U64(0x63DC359D8D231B78), U64(0xEC16CA8AEA98AD76),
   U64(0x5355F900C2A82DC7), U64(0x07FB9F855A997142), U64(0x5093417AA8A7ED5E), U64(0x7BCBC38DA25A7F3C),
   U64(0x19FC8A768CF4B6D4), U64(0x637A7780DECFC0D9), U64(0x8249A47AEE0E41F7), U64(0x79AD695501E7D1E8),
   U64(0x14ACBAF4777D5776), U64(0xF145B6BECCDEA195), U64(0xDABF2AC8201752FC), U64(0x24C3C94DF9C8D3F6),
   U64(0xBB6E2924F03912EA), U64(0x0CE26C0B95C980D9), U64(0xA49CD132BFBF7CC4), U64(0xE99D662AF4243939),
   U64(0x27E6AD7891165C3F), U64(0x8535F040B9744FF1), U64(0x54B3F4FA5F40D873), U64(0x72B12C32127FED2B),
   U64(0xEE954D3C7B411F47), U64(0x9A85AC909A24EAA1), U64(0x70AC4CD9F04F21F5), U64(0xF9B89D3E99A075C2),
   U64(0x87B3E2B2B5C907B1), U64(0xA366E5B8C54F48B8), U64(0xAE4A9346CC3F7CF2), U64(0x1920C04D47267BBD),
   U64(0x87BF02C6B49E2AE9), U64(0x092237AC237F3859), U64(0xFF07F64EF8ED14D0), U64(0x8DE8DCA9F03CC54E),
   U64(0x9C1633264DB49C89), U64(0xB3F22C3D0B0B38ED), U64(0x390E5FB44D01144B), U64(0x5BFEA5B4712768E9),
   U64(0x1E1032911FA78984), U64(0x9A74ACB964E78CB3), U64(0x4F80F7A035DAFB04), U64(0x6304D09A0B3738C4),
   U64(0x2171E64683023A08), U64(0x5B9B63EB9CEFF80C), U64(0x506AACF489889342), U64(0x1881AFC9A3A701D6),
   U64(0x6503080440750644), U64(0xDFD395339CDBF4A7), U64(0xEF927DBCF00C20F2), U64(0x7B32F7D1E03680EC),
   U64(0xB9FD7620E7316243), U64(0x05A7E8A57DB91B77), U64(0xB5889C6E15630A75), U64(0x4A750A09CE9573F7),
   U64(0xCF464CEC899A2F8A), U64(0xF538639CE705B824), U64(0x3C79A0FF5580EF7F), U64(0xEDE6C87F8477609D),
   U64(0x799E81F05BC93F31), U64(0x86536B8CF3428A8C), U64(0x97D7374C60087B73), U64(0xA246637CFF328532),
   U64(0x043FCAE60CC0EBA0), U64(0x920E449535DD359E), U64(0x70EB093B15B290CC), U64(0x73A1921916591CBD),
   U64(0x56436C9FE1A1AA8D), U64(0xEFAC4B70633B8F81), U64(0xBB215798D45DF7AF), U64(0x45F20042F24F1768),
   U64(0x930F80F4E8EB7462), U64(0xFF6712FFCFD75EA1), U64(0xAE623FD67468AA70), U64(0xDD2C5BC84BC8D8FC),
   U64(0x7EED120D54CF2DD9), U64(0x22FE545401165F1C), U64(0xC91800E98FB99929), U64(0x808BD68E6AC10365),
   U64(0xDEC468145B7605F6), U64(0x1BEDE3A3AEF53302), U64(0x43539603D6C55602), U64(0xAA969B5C691CCB7A),
   U64(0xA87832D392EFEE56), U64(0x65942C7B3C7E11AE), U64(0xDED2D633CAD004F6), U64(0x21F08570F420E565),
   U64(0xB415938D7DA94E3C), U64(0x91B859E59ECB6350), U64(0x10CFF333E0ED804A), U64(0x28AED140BE0BB7DD),
   U64(0xC5CC1D89724FA456), U64(0x5648F680F11A2741), U64(0x2D255069F0B7DAB3), U64(0x9BC5A38EF729ABD4),
   U64(0xEF2F054308F6A2BC), U64(0xAF2042F5CC5C2858), U64(0x480412BAB7F5BE2A), U64(0xAEF3AF4A563DFE43),
   U64(0x19AFE59AE451497F), U64(0x52593803DFF1E840), U64(0xF4F076E65F2CE6F0), U64(0x11379625747D5AF3),
   U64(0xBCE5D2248682C115), U64(0x9DA4243DE836994F), U64(0x066F70B33FE09017), U64(0x4DC4DE189B671A1C),
   U64(0x51039AB7712457C3), U64(0xC07A3F80C31FB4B4), U64(0xB46EE9C5E64A6E7C), U64(0xB3819A42ABE61C87),
   U64(0x21A007933A522A20), U64(0x2DF16F761598AA4F), U64(0x763C4A1371B368FD), U64(0xF793C46702E086A0),
   U64(0xD7288E012AEB8D31), U64(0xDE336A2A4BC1C44B), U64(0x0BF692B38D079F23), U64(0x2C604A7A177326B3),
   U64(0x4850E73E03EB6064), U64(0xCFC447F1E53C8E1B), U64(0xB05CA3F564268D99), U64(0x9AE182C8BC9474E8),
   U64(0xA4FC4BD4FC5558CA), U64(0xE755178D58FC4E76), U64(0x69B97DB1A4C03DFE), U64(0xF9B5B7C4ACC67C96),
   U64(0xFC6A82D64B8655FB), U64(0x9C684CB6C4D24417), U64(0x8EC97D2917456ED0), U64(0x6703DF9D2924E97E),
   U64(0xC547F57E42A7444E), U64(0x78E37644E7CAD29E), U64(0xFE9A44E9362F05FA), U64(0x08BD35CC38336615),
   U64(0x9315E5EB3A129ACE), U64(0x94061B871E04DF75), U64(0xDF1D9F9D784BA010), U64(0x3BBA57B68871B59D),
   U64(0xD2B7ADEEDED1F73F), U64(0xF7A255D83BC373F8), U64(0xD7F4F2448C0CEB81), U64(0xD95BE88CD210FFA7),
   U64(0x336F52F8FF4728E7), U64(0xA74049DAC312AC71), U64(0xA2F61BB6E437FDB5), U64(0x4F2A5CB07F6A35B3),
   U64(0x87D380BDA5BF7859), U64(0x16B9F7E06C453A21), U64(0x7BA2484C8A0FD54E), U64(0xF3A678CAD9A2E38C),
   U64(0x39B0BF7DDE437BA2), U64(0xFCAF55C1BF8A4424), U64(0x18FCF680573FA594), U64(0x4C0563B89F495AC3),
   U64(0x40E087931A00930D), U64(0x8CFFA9412EB642C1), U64(0x68CA39053261169F), U64(0x7A1EE967D27579E2),
   U64(0x9D1D60E5076F5B6F), U64(0x3810E399B6F65BA2), U64(0x32095B6D4AB5F9B1), U64(0x35CAB62109DD038A),
   U64(0xA90B24499FCFAFB1), U64(0x77A225A07CC2C6BD), U64(0x513E5E634C70E331), U64(0x4361C0CA3F692F12),
   U64(0xD941ACA44B20A45B), U64(0x528F7C8602C5807B), U64(0x52AB92BEB9613989), U64(0x9D1DFA2EFC557F73),
   U64(0x722FF175F572C348), U64(0x1D1260A51107FE97), U64(0x7A249A57EC0C9BA2), U64(0x04208FE9E8F7F2D6),
   U64(0x5A110C6058B920A0), U64(0x0CD9A497658A5698), U64(0x56FD23C8F9715A4C), U64(0x284C847B9D887AAE),
   U64(0x04FEABFBBDB619CB), U64(0x742E1E651C60BA83), U64(0x9A9632E65904AD3C), U64(0x881B82A13B51B9E2),
   U64(0x506E6744CD974924), U64(0xB0183DB56FFC6A79), U64(0x0ED9B915C66ED37E), U64(0x5E11E86D5873D484),
   U64(0xF678647E3519AC6E), U64(0x1B85D488D0F20CC5), U64(0xDAB9FE6525D89021), U64(0x0D151D86ADB73615),
   U64(0xA865A54EDCC0F019), U64(0x93C42566AEF98FFB), U64(0x99E7AFEABE000731), U64(0x48CBFF086DDF285A),
   U64(0x7F9B6AF1EBF78BAF), U64(0x58627E1A149BBA21), U64(0x2CD16E2ABD791E33), U64(0xD363EFF5F0977996),
   U64(0x0CE2A38C344A6EED), U64(0x1A804AADB9CFA741), U64(0x907F30421D78C5DE), U64(0x501F65EDB3034D07),
   U64(0x37624AE5A48FA6E9), U64(0x957BAF61700CFF4E), U64(0x3A6C27934E31188A), U64(0xD49503536ABCA345),
   U64(0x088E049589C432E0), U64(0xF943AEE7FEBF21B8), U64(0x6C3B8E3E336139D3), U64(0x364F6FFA464EE52E),
   U64(0xD60F6DCEDC314222), U64(0x56963B0DCA418FC0), U64(0x16F50EDF91E513AF), U64(0xEF1955914B609F93),
   U64(0x565601C0364E3228), U64(0xECB53939887E8175), U64(0xBAC7A9A18531294B), U64(0xB344C470397BBA52),
   U64(0x65D34954DAF3CEBD), U64(0xB4B81B3FA97511E2), U64(0xB422061193D6F6A7), U64(0x071582401C38434D),
   U64(0x7A13F18BBEDC4FF5), U64(0xBC4097B116C524D2), U64(0x59B97885E2F2EA28), U64(0x99170A5DC3115544),
   U64(0x6F423357E7C6A9F9), U64(0x325928EE6E6F8794), U64(0xD0E4366228B03343), U64(0x565C31F7DE89EA27),
   U64(0x30F5611484119414), U64(0xD873DB391292ED4F), U64(0x7BD94E1D8E17DEBC), U64(0xC7D9F16864A76E94),
   U64(0x947AE053EE56E63C), U64(0xC8C93882F9475F5F), U64(0x3A9BF55BA91F81CA), U64(0xD9A11FBB3D9808E4),
   U64(0x0FD22063EDC29FCA), U64(0xB3F256D8ACA0B0B9), U64(0xB03031A8B4516E84), U64(0x35DD37D5871448AF),
   U64(0xE9F6082B05542E4E), U64(0xEBFAFA33D7254B59), U64(0x9255ABB50D532280), U64(0xB9AB4CE57F2D34F3),
   U64(0x693501D628297551), U64(0xC62C58F97DD949BF), U64(0xCD454F8F19C5126A), U64(0xBBE83F4ECC2BDECB),
   U64(0xDC842B7E2819E230), U64(0xBA89142E007503B8), U64(0xA3BC941D0A5061CB), U64(0xE9F6760E32CD8021),
   U64(0x09C7E552BC76492F), U64(0x852F54934DA55CC9), U64(0x8107FCCF064FCF56), U64(0x098954D51FFF6580),
   U64(0x23B70EDB1955C4BF), U64(0xC330DE426430F69D), U64(0x4715ED43E8A45C0A), U64(0xA8D7E4DAB780A08D),
   U64(0x0572B974F03CE0BB), U64(0xB57D2E985E1419C7), U64(0xE8D9ECBE2CF3D73F), U64(0x2FE4B17170E59750),
   U64(0x11317BA87905E790), U64(0x7FBF21EC8A1F45EC), U64(0x1725CABFCB045B00), U64(0x964E915CD5E2B207),
   U64(0x3E2B8BCBF016D66D), U64(0xBE7444E39328A0AC), U64(0xF85B2B4FBCDE44B7), U64(0x49353FEA39BA63B1),
   U64(0x1DD01AAFCD53486A), U64(0x1FCA8A92FD719F85), U64(0xFC7C95D827357AFA), U64(0x18A6A990C8B35EBD),
   U64(0xCCCB7005C6B9C28D), U64(0x3BDBB92C43B17F26), U64(0xAA70B5B4F89695A2), U64(0xE94C39A54A98307F),
   U64(0xB7A0B174CFF6F36E), U64(0xD4DBA84729AF48AD), U64(0x2E18BC1AD9704A68), U64(0x2DE0966DAF2F8B1C),
   U64(0xB9C11D5B1E43A07E), U64(0x64972D68DEE33360), U64(0x94628D38D0C20584), U64(0xDBC0D2B6AB90A559),
   U64(0xD2733C4335C6A72F), U64(0x7E75D99D94A70F4D), U64(0x6CED1983376FA72B), U64(0x97FCAACBF030BC24),
   U64(0x7B77497B32503B12), U64(0x8547EDDFB81CCB94), U64(0x79999CDFF70902CB), U64(0xCFFE1939438E9B24),
   U64(0x829626E3892D95D7), U64(0x92FAE24291F2B3F1), U64(0x63E22C147B9C3403), U64(0xC678B6D860284A1C),
   U64(0x5873888850659AE7), U64(0x0981DCD296A8736D), U64(0x9F65789A6509A440), U64(0x9FF38FED72E9052F),
   U64(0xE479EE5B9930578C), U64(0xE7F28ECD2D49EECD), U64(0x56C074A581EA17FE), U64(0x5544F7D774B14AEF),
   U64(0x7B3F0195FC6F290F), U64(0x12153635B2C0CF57), U64(0x7F5126DBBA5E0CA7), U64(0x7A76956C3EAFB413),
   U64(0x3D5774A11D31AB39), U64(0x8A1B083821F40CB4), U64(0x7B4A38E32537DF62), U64(0x950113646D1D6E03),
   U64(0x4DA8979A0041E8A9), U64(0x3BC36E078F7515D7), U64(0x5D0A12F27AD310D1), U64(0x7F9D1A2E1EBE1327),
   U64(0xDA3A361B1C5157B1), U64(0xDCDD7D20903D0C25), U64(0x36833336D068F707), U64(0xCE68341F79893389),
   U64(0xAB9090168DD05F34), U64(0x43954B3252DC25E5), U64(0xB438C2B67F98E5E9), U64(0x10DCD78E3851A492),
   U64(0xDBC27AB5447822BF), U64(0x9B3CDB65F82CA382), U64(0xB67B7896167B4C84), U64(0xBFCED1B0048EAC50),
   U64(0xA9119B60369FFEBD), U64(0x1FFF7AC80904BF45), U64(0xAC12FB171817EEE7), U64(0xAF08DA9177DDA93D),
   U64(0x1B0CAB936E65C744), U64(0xB559EB1D04E5E932), U64(0xC37B45B3F8D6F2BA), U64(0xC3A9DC228CAAC9E9),
   U64(0xF3B8B6675A6507FF), U64(0x9FC477DE4ED681DA), U64(0x67378D8ECCEF96CB), U64(0x6DD856D94D259236),
   U64(0xA319CE15B0B4DB31), U64(0x073973751F12DD5E), U64(0x8A8E849EB32781A5), U64(0xE1925C71285279F5),
   U64(0x74C04BF1790C0EFE), U64(0x4DDA48153C94938A), U64(0x9D266D6A1CC0542C), U64(0x7440FB816508C4FE),
   U64(0x13328503DF48229F), U64(0xD6BF7BAEE43CAC40), U64(0x4838D65F6EF6748F), U64(0x1E152328F3318DEA),
   U64(0x8F8419A348F296BF), U64(0x72C8834A5957B511), U64(0xD7A023A73260B45C), U64(0x94EBC8ABCFB56DAE),
   U64(0x9FC10D0F989993E0), U64(0xDE68A2355B93CAE6), U64(0xA44CFE79AE538BBE), U64(0x9D1D84FCCE371425),
   U64(0x51D2B1AB2DDFB636), U64(0x2FD7E4B9E72CD38C), U64(0x65CA5B96B7552210), U64(0xDD69A0D8AB3B546D),
   U64(0x604D51B25FBF70E2), U64(0x73AA8A564FB7AC9E), U64(0x1A8C1E992B941148), U64(0xAAC40A2703D9BEA0),
   U64(0x764DBEAE7FA4F3A6), U64(0x1E99B96E70A9BE8B), U64(0x2C5E9DEB57EF4743), U64(0x3A938FEE32D29981),
   U64(0x26E6DB8FFDF5ADFE), U64(0x469356C504EC9F9D), U64(0xC8763C5B08D1908C), U64(0x3F6C6AF859D80055),
   U64(0x7F7CC39420A3A545), U64(0x9BFB227EBDF4C5CE), U64(0x89039D79D6FC5C5C), U64(0x8FE88B57305E2AB6),
   U64(0xA09E8C8C35AB96DE), U64(0xFA7E393983325753), U64(0xD6B6D0ECC617C699), U64(0xDFEA21EA9E7557E3),
   U64(0xB67C1FA481680AF8), U64(0xCA1E3785A9E724E5), U64(0x1CFC8BED0D681639), U64(0xD18D8549D140CAEA),
   U64(0x4ED0FE7E9DC91335), U64(0xE4DBF0634473F5D2), U64(0x1761F93A44D5AEFE), U64(0x53898E4C3910DA55),
   U64(0x734DE8181F6EC39A), U64(0x2680B122BAA28D97), U64(0x298AF231C85BAFAB), U64(0x7983EED3740847D5),
   U64(0x66C1A2A1A60CD889), U64(0x9E17E49642A3E4C1), U64(0xEDB454E7BADC0805), U64(0x50B704CAB602C329),
   U64(0x4CC317FB9CDDD023), U64(0x66B4835D9EAFEA22), U64(0x219B97E26FFC81BD), U64(0x261E4E4C0A333A9D),
   U64(0x1FE2CCA76517DB90), U64(0xD7504DFA8816EDBB), U64(0xB9571FA04DC089C8), U64(0x1DDC0325259B27DE),
   U64(0xCF3F4688801EB9AA), U64(0xF4F5D05C10CAB243), U64(0x38B6525C21A42B0E), U64(0x36F60E2BA4FA6800),
   U64(0xEB3593803173E0CE), U64(0x9C4CD6257C5A3603), U64(0xAF0C317D32ADAA8A), U64(0x258E5A80C7204C4B),
   U64(0x8B889D624D44885D), U64(0xF4D14597E660F855), U64(0xD4347F66EC8941C3), U64(0xE699ED85B0DFB40D),
   U64(0x2472F6207C2D0484), U64(0xC2A1E7B5B459AEB5), U64(0xAB4F6451CC1D45EC), U64(0x63767572AE3D6174),
   U64(0xA59E0BD101731A28), U64(0x116D0016CB948F09), U64(0x2CF9C8CA052F6E9F), U64(0x0B090A7560A968E3),
   U64(0xABEEDDB2DDE06FF1), U64(0x58EFC10B06A2068D), U64(0xC6E57A78FBD986E0), U64(0x2EAB8CA63CE802D7),
   U64(0x14A195640116F336), U64(0x7C0828DD624EC390), U64(0xD74BBE77E6116AC7), U64(0x804456AF10F5FB53),
   U64(0xEBE9EA2ADF4321C7), U64(0x03219A39EE587A30), U64(0x49787FEF17AF9924), U64(0xA1E9300CD8520548),
   U64(0x5B45E522E4B1B4EF), U64(0xB49C3B3995091A36), U64(0xD4490AD526F14431), U64(0x12A8F216AF9418C2),
   U64(0x001F837CC7350524), U64(0x1877B51E57A764D5), U64(0xA2853B80F17F58EE), U64(0x993E1DE72D36D310),
   U64(0xB3598080CE64A656), U64(0x252F59CF0D9F04BB), U64(0xD23C8E176D113600), U64(0x1BDA0492E7E4586E),
   U64(0x21E0BD5026C619BF), U64(0x3B097ADAF088F94E), U64(0x8D14DEDB30BE846E), U64(0xF95CFFA23AF5F6F4),
   U64(0x3871700761B3F743), U64(0xCA672B91E9E4FA16), U64(0x64C8E531BFF53B55), U64(0x241260ED4AD1E87D),
   U64(0x106C09B972D2E822), U64(0x7FBA195410E5CA30), U64(0x7884D9BC6CB569D8), U64(0x0647DFEDCD894A29),
   U64(0x63573FF03E224774), U64(0x4FC8E9560F91B123), U64(0x1DB956E450275779), U64(0xB8D91274B9E9D4FB),
   U64(0xA2EBEE47E2FBFCE1), U64(0xD9F1F30CCD97FB09), U64(0xEFED53D75FD64E6B), U64(0x2E6D02C36017F67F),
   U64(0xA9AA4D20DB084E9B), U64(0xB64BE8D8B25396C1), U64(0x70CB6AF7C2D5BCF0), U64(0x98F076A4F7A2322E),
   U64(0xBF84470805E69B5F), U64(0x94C3251F06F90CF3), U64(0x3E003E616A6591E9), U64(0xB925A6CD0421AFF3),
   U64(0x61BDD1307C66E300), U64(0xBF8D5108E27E0D48), U64(0x240AB57A8B888B20), U64(0xFC87614BAF287E07),
   U64(0xEF02CDD06FFDB432), U64(0xA1082C0466DF6C0A), U64(0x8215E577001332C8), U64(0xD39BB9C3A48DB6CF),
   U64(0x2738259634305C14), U64(0x61CF4F94C97DF93D), U64(0x1B6BACA2AE4E125B), U64(0x758F450C88572E0B),
   U64(0x959F587D507A8359), U64(0xB063E962E045F54D), U64(0x60E8ED72C0DFF5D1), U64(0x7B64978555326F9F),
   U64(0xFD080D236DA814BA), U64(0x8C90FD9B083F4558), U64(0x106F72FE81E2C590), U64(0x7976033A39F7D952),
   U64(0xA4EC0132764CA04B), U64(0x733EA705FAE4FA77), U64(0xB4D8F77BC3E56167), U64(0x9E21F4F903B33FD9),
   U64(0x9D765E419FB69F6D), U64(0xD30C088BA61EA5EF), U64(0x5D94337FBFAF7F5B), U64(0x1A4E4822EB4D7A59),
   U64(0x6FFE73E81B637FB3), U64(0xDDF957BC36D8B9CA), U64(0x64D0E29EEA8838B3), U64(0x08DD9BDFD96B9F63),
   U64(0x087E79E5A57D1D13), U64(0xE328E230E3E2B3FB), U64(0x1C2559E30F0946BE), U64(0x720BF5F26F4D2EAA),
   U64(0xB0774D261CC609DB), U64(0x443F64EC5A371195), U64(0x4112CF68649A260E), U64(0xD813F2FAB7F5C5CA),
   U64(0x660D3257380841EE), U64(0x59AC2C7873F910A3), U64(0xE846963877671A17), U64(0x93B633ABFA3469F8),
   U64(0xC0C0F5A60EF4CDCF), U64(0xCAF21ECD4377B28C), U64(0x57277707199B8175), U64(0x506C11B9D90E8B1D),
   U64(0xD83CC2687A19255F), U64(0x4A29C6465A314CD1), U64(0xED2DF21216235097), U64(0xB5635C95FF7296E2),
   U64(0x22AF003AB672E811), U64(0x52E762596BF68235), U64(0x9AEBA33AC6ECC6B0), U64(0x944F6DE09134DFB6),
   U64(0x6C47BEC883A7DE39), U64(0x6AD047C430A12104), U64(0xA5B1CFDBA0AB4067), U64(0x7C45D833AFF07862),
   U64(0x5092EF950A16DA0B), U64(0x9338E69C052B8E7B), U64(0x455A4B4CFE30E3F5), U64(0x6B02E63195AD0CF8),
   U64(0x6B17B224BAD6BF27), U64(0xD1E0CCD25BB9C169), U64(0xDE0C89A556B9AE70), U64(0x50065E535A213CF6),
   U64(0x9C1169FA2777B874), U64(0x78EDEFD694AF1EED), U64(0x6DC93D9526A50E68), U64(0xEE97F453F06791ED),
   U64(0x32AB0EDB696703D3), U64(0x3A6853C7E70757A7), U64(0x31865CED6120F37D), U64(0x67FEF95D92607890),
   U64(0x1F2B1D1F15F6DC9C), U64(0xB69E38A8965C6B65), U64(0xAA9119FF184CCCF4), U64(0xF43C732873F24C13),
   U64(0xFB4A3D794A9A80D2), U64(0x3550C2321FD6109C), U64(0x371F77E76BB8417E), U64(0x6BFA9AAE5EC05779),
   U64(0xCD04F3FF001A4778), U64(0xE3273522064480CA), U64(0x9F91508BFFCFC14A), U64(0x049A7F41061A9E60),
   U64(0xFCB6BE43A9F2FE9B), U64(0x08DE8A1C7797DA9B), U64(0x8F9887E6078735A1), U64(0xB5B4071DBFC73A66),
   U64(0x230E343DFBA08D33), U64(0x43ED7F5A0FAE657D), U64(0x3A88A0FBBCB05C63), U64(0x21874B8B4D2DBC4F),
   U64(0x1BDEA12E35F6A8C9), U64(0x53C065C6C8E63528), U64(0xE34A1D250E7A8D6B), U64(0xD6B04D3B7651DD7E),
   U64(0x5E90277E7CB39E2D), U64(0x2C046F22062DC67D), U64(0xB10BB459132D0A26), U64(0x3FA9DDFB67E2F199),
   U64(0x0E09B88E1914F7AF), U64(0x10E8B35AF3EEAB37), U64(0x9EEDECA8E272B933), U64(0xD4C718BC4AE8AE5F),
   U64(0x81536D601170FC20), U64(0x91B534F885818A06), U64(0xEC8177F83F900978), U64(0x190E714FADA5156E),
   U64(0xB592BF39B0364963), U64(0x89C350C893AE7DC1), U64(0xAC042E70F8B383F2), U64(0xB49B52E587A1EE60),
   U64(0xFB152FE3FF26DA89), U64(0x3E666E6F69AE2C15), U64(0x3B544EBE544C19F9), U64(0xE805A1E290CF2456),
   U64(0x24B33C9D7ED25117), U64(0xE74733427B72F0C1), U64(0x0A804D18B7097475), U64(0x57E3306D881EDB4F),
   U64(0x4AE7D6A36EB5DBCB), U64(0x2D8D5432157064C8), U64(0xD1E649DE1E7F268B), U64(0x8A328A1CEDFE552C),
   U64(0x07A3AEC79624C7DA), U64(0x84547DDC3E203C94), U64(0x990A98FD5071D263), U64(0x1A4FF12616EEFC89),
   U64(0xF6F7FD1431714200), U64(0x30C05B1BA332F41C), U64(0x8D2636B81555A786), U64(0x46C9FEB55D120902),
   U64(0xCCEC0A73B49C9921), U64(0x4E9D2827355FC492), U64(0x19EBB029435DCB0F), U64(0x4659D2B743848A2C),
   U64(0x963EF2C96B33BE31), U64(0x74F85198B05A2E7D), U64(0x5A0F544DD2B1FB18), U64(0x03727073C2E134B1),
   U64(0xC7F6AA2DE59AEA61), U64(0x352787BAA0D7C22F), U64(0x9853EAB63B5E0B35), U64(0xABBDCDD7ED5C0860),
   U64(0xCF05DAF5AC8D77B0), U64(0x49CAD48CEBF4A71E), U64(0x7A4C10EC2158C4A6), U64(0xD9E92AA246BF719E),
   U64(0x13AE978D09FE5557), U64(0x730499AF921549FF), U64(0x4E4B705B92903BA4), U64(0xFF577222C14F0A3A),
   U64(0x55B6344CF97AAFAE), U64(0xB862225B055B6960), U64(0xCAC09AFBDDD2CDB4), U64(0xDAF8E9829FE96B5F),
   U64(0xB5FDFC5D3132C498), U64(0x310CB380DB6F7503), U64(0xE87FBB46217A360E), U64(0x2102AE466EBB1148),
   U64(0xF8549E1A3AA5E00D), U64(0x07A69AFDCC42261A), U64(0xC4C118BFE78FEAAE), U64(0xF9F4892ED96BD438),
   U64(0x1AF3DBE25D8F45DA), U64(0xF5B4B0B0D2DEEEB4), U64(0x962ACEEFA82E1C84), U64(0x046E3ECAAF453CE9),
   U64(0xF05D129681949A4C), U64(0x964781CE734B3C84), U64(0x9C2ED44081CE5FBD), U64(0x522E23F3925E319E),
   U64(0x177E00F9FC32F791), U64(0x2BC60A63A6F3B3F2), U64(0x222BBFAE61725606), U64(0x486289DDCC3D6780),
   U64(0x7DC7785B8EFDFC80), U64(0x8AF38731C02BA980), U64(0x1FAB64EA29A2DDF7), U64(0xE4D9429322CD065A),
   U64(0x9DA058C67844F20C), U64(0x24C0E332B70019B0), U64(0x233003B5A6CFE6AD), U64(0xD586BD01C5C217F6),
   U64(0x5E5637885F29BC2B), U64(0x7EBA726D8C94094B), U64(0x0A56A5F0BFE39272), U64(0xD79476A84EE20D06),
   U64(0x9E4C1269BAA4BF37), U64(0x17EFEE45B0DEE640), U64(0x1D95B0A5FCF90BC6), U64(0x93CBE0B699C2585D),
   U64(0x65FA4F227A2B6D79), U64(0xD5F9E858292504D5), U64(0xC2B5A03F71471A6F), U64(0x59300222B4561E00),
   U64(0xCE2F8642CA0712DC), U64(0x7CA9723FBB2E8988), U64(0x2785338347F2BA08), U64(0xC61BB3A141E50E8C),
   U64(0x150F361DAB9DEC26), U64(0x9F6A419D382595F4), U64(0x64A53DC924FE7AC9), U64(0x142DE49FFF7A7C3D),
   U64(0x0C335248857FA9E7), U64(0x0A9C32D5EAE45305), U64(0xE6C42178C4BBB92E), U64(0x71F1CE2490D20B07),
   U64(0xF1BCC3D275AFE51A), U64(0xE728E8C83C334074), U64(0x96FBF83A12884624), U64(0x81A1549FD6573DA5),
   U64(0x5FA7867CAF35E149), U64(0x56986E2EF3ED091B), U64(0x917F1DD5F8886C61), U64(0xD20D8C88C8FFE65F),
   U64(0x31D71DCE64B2C310), U64(0xF165B587DF898190), U64(0xA57E6339DD2CF3A0), U64(0x1EF6E6DBB1961EC9),
   U64(0x70CC73D90BC26E24), U64(0xE21A6B35DF0C3AD7), U64(0x003A93D8B2806962), U64(0x1C99DED33CB890A1),
   U64(0xCF3145DE0ADD4289), U64(0xD0E4427A5514FB72), U64(0x77C621CC9FB3A483), U64(0x67A34DAC4356550B),
   U64(0xF8D626AAAF278509),
};

uint64 *RandomPiece     =Random64;
uint64 *RandomCastle    =Random64+768;
uint64 *RandomEnPassant =Random64+772;
uint64 *RandomTurn      =Random64+780;


uint64
hash (int moveNr)
{
    int r, f, p_enc, squareNr, pieceGroup;
    uint64 key=0, holdingsKey=0, Zobrist;
    VariantClass v = gameInfo.variant;

    switch(v) {
	case VariantNormal:
	case VariantFischeRandom: // compatible with normal
	case VariantNoCastle:
	case VariantXiangqi: // for historic reasons; does never collide anyway because of other King type
	    break;
	case VariantGiveaway: // in opening same as suicide
	    key += VariantSuicide;
	    break;
	case VariantGothic: // these are special cases of CRC, and can share book
	case VariantCapablanca:
	    v = VariantCapaRandom;
	default:
	    key += v; // variant type incorporated in key to allow mixed books without collisions
    }

    for(f=0; f<BOARD_WIDTH; f++){
        for(r=0; r<BOARD_HEIGHT;r++){
            ChessSquare p = boards[moveNr][r][f];
	    if(f == BOARD_LEFT-1 || f == BOARD_RGHT) continue; // between board and holdings
            if(p != EmptySquare){
		    int j = (int)p, promoted = 0;
		    j -= (j >= (int)BlackPawn) ? (int)BlackPawn :(int)WhitePawn;
		    if(j >= WhitePBishop && j != WhiteKing) promoted++, j -= WhiteTokin;
		    if(j > (int)WhiteQueen) j++;  // make space for King
		    if(j > (int) WhiteKing) j = (int)WhiteQueen + 1;
		    p_enc = 2*j + ((int)p < (int)BlackPawn);
		    // holdings squares get nmbers immediately after board; first left, then right holdings
		    if(f == BOARD_LEFT-2) squareNr = (BOARD_RGHT - BOARD_LEFT)*BOARD_HEIGHT + r; else
		    if(f == BOARD_RGHT+1) squareNr = (BOARD_RGHT - BOARD_LEFT + 1)*BOARD_HEIGHT + r; else
		    squareNr = (BOARD_RGHT - BOARD_LEFT)*r + (f - BOARD_LEFT);
		    // note that in normal Chess squareNr < 64 and p_enc < 12. The following code
		    // maps other pieces and squares in this range, and then modify the corresponding
		    // Zobrist random by rotating its bitpattern according to what the piece really was.
		    pieceGroup = p_enc / 12;
		    p_enc      = p_enc % 12;
		    Zobrist = RandomPiece[64*p_enc + (squareNr & 63)];
		    if(pieceGroup & 4) Zobrist *= 987654321;
		    switch(pieceGroup & 3) {
			case 1: // pieces 5-10 (FEACWM)
				Zobrist = (Zobrist << 16) ^ (Zobrist >> 48);
				break;
			case 2: // pieces 11-16 (OHIJGD)
				Zobrist = (Zobrist << 32) ^ (Zobrist >> 32);
				break;
			case 3: // pieces 17-20 (VLSU)
				Zobrist = (Zobrist << 48) ^ (Zobrist >> 16);
				break;
		    }
		    if(promoted) Zobrist ^= 123456789*RandomPiece[squareNr & 63];
		    if(squareNr &  64) Zobrist = (Zobrist << 8) ^ (Zobrist >> 56);
		    if(squareNr & 128) Zobrist = (Zobrist << 4) ^ (Zobrist >> 60);
		    // holdings have separate (additive) key, to encode presence of multiple pieces on same square
		    if(f == BOARD_LEFT-2) holdingsKey += Zobrist * boards[moveNr][r][f+1]; else
		    if(f == BOARD_RGHT+1) holdingsKey += Zobrist * boards[moveNr][r][f-1]; else
                key ^= Zobrist;
            }
        }
    }

    if(boards[moveNr][CASTLING][2] != NoRights) {
	if(boards[moveNr][CASTLING][0] != NoRights) key^=RandomCastle[0];
	if(boards[moveNr][CASTLING][1] != NoRights) key^=RandomCastle[1];
    }
    if(boards[moveNr][CASTLING][5] != NoRights) {
	if(boards[moveNr][CASTLING][3] != NoRights) key^=RandomCastle[2];
	if(boards[moveNr][CASTLING][4] != NoRights) key^=RandomCastle[3];
    }

    f = boards[moveNr][EP_STATUS];
    if(f >= 0 && f < 8){
        if(!WhiteOnMove(moveNr)){
	    // the test for neighboring Pawns might not be needed,
	    // as epStatus already kept track of it, but better safe than sorry.
            if((f>0 && boards[moveNr][3][f-1]==BlackPawn)||
               (f<7 && boards[moveNr][3][f+1]==BlackPawn)){
                key^=RandomEnPassant[f];
            }
        }else{
            if((f>0 && boards[moveNr][4][f-1]==WhitePawn)||
               (f<7 && boards[moveNr][4][f+1]==WhitePawn)){
                key^=RandomEnPassant[f];
            }
        }
    }

    if(WhiteOnMove(moveNr)){
        key^=RandomTurn[0];
    }
    return key + holdingsKey;
}

#define MOVE_BUF 100

// fs routines read from memory buffer if no file specified

static unsigned char *memBuf, *memPtr;
static int bufSize;
Boolean mcMode;

int
fsseek (FILE *f, int n, int mode)
{
    if(f) return fseek(f, n, mode);
    if(mode == SEEK_SET) memPtr = memBuf + n; else
    if(mode == SEEK_END) memPtr = memBuf + 16*bufSize + n;
    return memPtr < memBuf || memPtr > memBuf + 16*bufSize;
}

int
fstell (FILE *f)
{
  if(f) return ftell(f);
  return memPtr - memBuf;
}

int
fsgetc (FILE *f)
{
  if(f) return fgetc(f);
  if(memPtr >= memBuf + 16*bufSize) return EOF;
  return *memPtr++ ;
}

int
int_from_file (FILE *f, int l, uint64 *r)
{
    int i,c;
    for(i=0;i<l;i++){
        c=fsgetc(f);
        if(c==EOF){
            return 1;
        }
        (*r)=((*r)<<8)+c;
    }
    return 0;
}

int
entry_from_file (FILE *f, entry_t *entry)
{
    int ret;
    uint64 r;
    if(!f) { *entry = *(entry_t*) memPtr; memPtr += 16; return 0; }
    ret=int_from_file(f,8,&r);
    if(ret) return 1;
    entry->key=r;
    ret=int_from_file(f,2,&r);
    if(ret) return 1;
    entry->move=r;
    ret=int_from_file(f,2,&r);
    if(ret) return 1;
    entry->weight=r;
    ret=int_from_file(f,2,&r);
    if(ret) return 1;
    entry->learnCount=r;
    ret=int_from_file(f,2,&r);
    if(ret) return 1;
    entry->learnPoints=r;
    return 0;
}

int
find_key (FILE *f, uint64 key, entry_t *entry)
{
    int first, last, middle;
    entry_t last_entry,middle_entry;
    first=-1;
    if(fsseek(f,-16,SEEK_END)){
        *entry=entry_none;
        entry->key=key+1; //hack
        return -1;
    }
    last=fstell(f)/16;
    entry_from_file(f,&last_entry);
    while(1){
        if(last-first==1){
            *entry=last_entry;
            return last;
        }
        middle=(first+last)/2;
        fsseek(f,16*middle,SEEK_SET);
        entry_from_file(f,&middle_entry);
        if(key<=middle_entry.key){
            last=middle;
            last_entry=middle_entry;
        }else{
            first=middle;
        }
    }
}

static int xStep[] = { 0, 1, 1, 1, 0,-1,-1,-1 };
static int yStep[] = { 1, 1, 0,-1,-1,-1, 0, 1 };

void
move_to_string (char move_s[20], uint16 move)
{
    int f,fr,ff,t,tr,tf,p;
    int width = BOARD_RGHT - BOARD_LEFT, size; // allow for alternative board formats

    size = width * BOARD_HEIGHT;
    p    = move / (size*size);
    move = move % (size*size);
    f  = move / size;
    fr = f / width;
    ff = f % width;
    t  = move % size;
    tr = t / width;
    tf = t % width;
    snprintf(move_s, 9, "%c%d%c%d", ff + 'a', fr + 1 - (BOARD_HEIGHT == 10), tf + 'a', tr + 1 - (BOARD_HEIGHT == 10));

    if(IS_SHOGI(gameInfo.variant) && p) {
	if(p == 2) p = 10;     // Lion moves, for boards so big that 10 is out of range
	else if(p != 7) p = 8; // use '+' for all others that do not explicitly defer
    }

    // kludge: encode drops as special promotion code
    if(gameInfo.holdingsSize && p == 9) {
	move_s[0] = f + '@'; // from square encodes piece type
	move_s[1] = '@';     // drop symbol
	p = 0;
    } else if(p == 10) { // decode Lion move
	int i = t & 7, j = t >> 3 & 7;
	tf = ff + xStep[i] + xStep[j]; tr = fr + yStep[i] + yStep[j]; // calculate true to-square
	snprintf(move_s, 20, "%c%d%c%d,%c%d%c%d", ff + 'a', fr + 1 - (BOARD_HEIGHT == 10),
						  ff + xStep[i] + 'a', fr + yStep[i] + 1 - (BOARD_HEIGHT == 10),
						  ff + xStep[i] + 'a', fr + yStep[i] + 1 - (BOARD_HEIGHT == 10),
						  tf + 'a', tr + 1 - (BOARD_HEIGHT == 10) );
	p = 0;
    }

    // add promotion piece, if any
    if(p){
	int len = strlen(move_s);
        move_s[len] = promote_pieces[p];
        move_s[len+1] = '\0';
    }

    if(gameInfo.variant != VariantNormal) return;

    // correct FRC-style castlings in variant normal.
    // [HGM] This is buggy code! e1h1 could very well be a normal R or Q move.
    if(!strcmp(move_s,"e1h1")){
      safeStrCpy(move_s,"e1g1", 6);
    }else  if(!strcmp(move_s,"e1a1")){
      safeStrCpy(move_s,"e1c1", 6);
    }else  if(!strcmp(move_s,"e8h8")){
      safeStrCpy(move_s,"e8g8", 6);
    }else  if(!strcmp(move_s,"e8a8")){
      safeStrCpy(move_s,"e8c8", 6);
    }
}

int
GetBookMoves (FILE *f, int moveNr, entry_t entries[], int max)
{   // retrieve all entries for given position from book in 'entries', return number.
    entry_t entry;
    int offset;
    uint64 key;
    int count;
    int ret;

    key = hash(moveNr);
    if(appData.debugMode) fprintf(debugFP, "book key = %08x%08x\n", (unsigned int)(key>>32), (unsigned int)key);

    offset=find_key(f, key, &entry);
    if(entry.key != key) {
	  return FALSE;
    }
    entries[0] = entry;
    count=1;
    fsseek(f, 16*(offset+1), SEEK_SET);
    while(1){
        ret=entry_from_file(f, &entry);
        if(ret){
            break;
        }
        if(entry.key != key){
            break;
        }
        if(count == max) break;
        entries[count++] = entry;
    }
    return count;
}

static int dirty;

int
ReadFromBookFile (int moveNr, char *book, entry_t entries[])
{   // retrieve all entries for given position from book in 'entries', return number.
    static FILE *f = NULL;
    static char curBook[MSG_SIZ];

    if(book == NULL) return -1;
    if(dirty) { if(f) fclose(f); dirty = 0; f = NULL; }
    if(!f || strcmp(book, curBook)){ // keep book file open until book changed
	strncpy(curBook, book, MSG_SIZ);
	if(f) fclose(f);
	f = fopen(book,"rb");
    }
    if(!f){
	DisplayError(_("Polyglot book not valid"), 0);
	appData.usePolyglotBook = FALSE;
	return -1;
    }

    return GetBookMoves(f, moveNr, entries, MOVE_BUF);
}

// next three made into subroutines to facilitate future changes in storage scheme (e.g. 2 x 3 bytes)

static int
wins(entry_t *e)
{
    return e->learnPoints;
}

static int
losses(entry_t *e)
{
    return e->learnCount;
}

static void
CountMove (entry_t *e, int result)
{
    switch(result) {
      case 0: e->learnCount ++; break;
      case 1: e->learnCount ++; // count draw as win + loss
      case 2: e->learnPoints ++; break;
    }
}

#define MERGESIZE 2048
#define HASHSIZE  1024*1024*4

entry_t *memBook, *hashTab, *mergeBuf;
int bookSize=1, mergeSize=1, mask = HASHSIZE-1;

void
InitMemBook ()
{
    static int initDone = FALSE;
    if(initDone) return;
    memBook  = (entry_t *) calloc(1024*1024, sizeof(entry_t));
    hashTab  = (entry_t *) calloc(HASHSIZE,  sizeof(entry_t));
    mergeBuf = (entry_t *) calloc(MERGESIZE+5, sizeof(entry_t));
    memBook[0].key  = -1LL;
    mergeBuf[0].key = -1LL;
    initDone = TRUE;
}

char *
MCprobe (moveNr)
{
    int count, count2, games, i, choice=0;
    entry_t entries[MOVE_BUF];
    float nominal[MOVE_BUF], tot, deficit, max, min;
    static char move_s[6];

    InitMemBook();
    memBuf = (unsigned char*) memBook; bufSize = bookSize;   // in MC mode book resides in memory
    count = GetBookMoves(NULL, moveNr, entries, MOVE_BUF);
    if(count < 0) count = 0; // don't care about miss yet
    memBuf = (unsigned char*) mergeBuf; bufSize = mergeSize; // there could be moves still waiting to be merged
    count2 = count + GetBookMoves(NULL, moveNr, entries+count, MOVE_BUF - count);
    if(appData.debugMode) fprintf(debugFP, "MC probe: %d/%d (%d+%d)\n", count, count2,bookSize,mergeSize);
    if(!count2) return NULL;
    tot = games = 0;
    for(i=0; i<count2; i++) {
	float w = wins(entries+i) + 10., l = losses(entries+i) + 10.;
	float h = (w*w*w*w + 22500.*w*w) / (l*l*l*l + 22500.*l*l);
	tot += nominal[i] = h;
	games += wins(entries+i) + losses(entries+i);
    }
    tot = games / tot; max = min = 0;
    for(i=0; i<count2; i++) {
	nominal[i] *= tot; // normalize so they sum to games
	deficit = nominal[i] - (wins(entries+i) + losses(entries+i));
	if(deficit > max) max = deficit, choice = i;
	if(deficit < min) min = deficit;
    } // note that a single move will never be underplayed
    if(max - min > 0.5*sqrt(nominal[choice])) { // if one of the listed moves is significantly under-played, play it now.
	move_to_string(move_s, entries[choice].move);
	if(appData.debugMode) fprintf(debugFP, "book move field = %d\n", entries[choice].move);
	return move_s;
    }
    return NULL; // otherwise fake book miss to force engine think, hoping for hitherto unplayed move.
}

char
*ProbeBook (int moveNr, char *book)
{   //
    entry_t entries[MOVE_BUF];
    int count;
    int i, j;
    static char move_s[6];
    int total_weight;

    if(moveNr >= 2*appData.bookDepth) return NULL;
    if(mcMode) return MCprobe(moveNr);

    if((count = ReadFromBookFile(moveNr, book, entries)) <= 0) return NULL; // no book, or no hit

    if(appData.bookStrength != 50) { // transform weights
        double power = 0, maxWeight = 0.0;
        if(appData.bookStrength) power = (100.-appData.bookStrength)/appData.bookStrength;
        for(i=0; i<count; i++) if(entries[i].weight > maxWeight) maxWeight = entries[i].weight;
        for(i=0; i<count; i++){
            double weight = entries[i].weight / maxWeight;
	      if(weight > 0)
                entries[i].weight = appData.bookStrength || weight == 1.0 ? 1e4*exp(power * log(weight)) + 0.5 : 0.0;
        }
    }
    total_weight = 0;
    for(i=0; i<count; i++){
        total_weight += entries[i].weight;
    }
    if(total_weight == 0) return NULL; // force book miss rather than playing moves with weight 0.
    j = (random() & 0xFFF) * total_weight >> 12; // create random < total_weight
    total_weight = 0;
    for(i=0; i<count; i++){
        total_weight += entries[i].weight;
	if(total_weight > j) break;
    }
    if(i >= count) DisplayFatalError(_("Book Fault"), 0, 1); // safety catch, cannot happen
    move_to_string(move_s, entries[i].move);
    if(appData.debugMode) fprintf(debugFP, "book move field = %d\n", entries[i].move);

    return move_s;
}

extern char yy_textstr[];
entry_t lastEntries[MOVE_BUF];

char *
MovesToText(int count, entry_t *entries)
{
	int i, totalWeight = 0;
	char algMove[12];
	char *p = (char*) malloc(40*count+1);
	for(i=0; i<count; i++) totalWeight += entries[i].weight;
	*p = 0;
	for(i=0; i<count; i++) {
	    char buf[MSG_SIZ], c1, c2, c3; int i1, i2, i3;
	    move_to_string(algMove, entries[i].move);
	    if(sscanf(algMove, "%c%d%*c%*d,%c%d%c%d", &c1, &i1, &c2, &i2, &c3, &i3) == 6)
		snprintf(algMove, 12, "%c%dx%c%d-%c%d", c1, i1, c2, i2, c3, i3); // cast double-moves in format SAN parser will understand
	    else if(sscanf(algMove, "%c%d%c%d%c", &c1, &i1, &c2, &i2, &c3) >= 4) {
		CoordsToAlgebraic(boards[currentMove], PosFlags(currentMove), i1-ONE+'0', c1-AAA, i2-ONE+'0', c2-AAA, c3, algMove);
	    }
	    buf[0] = NULLCHAR;
	    if(entries[i].learnCount || entries[i].learnPoints)
		snprintf(buf, MSG_SIZ, " {%d/%d}", entries[i].learnPoints, entries[i].learnCount);
	    snprintf(p+strlen(p), 40, "%5.1f%% %5d %s%s\n", 100*entries[i].weight/(totalWeight+0.001),
					entries[i].weight, algMove, buf);
//lastEntries[i] = entries[i];
	}
	return p;
}

static int
CoordsToMove (int fromX, int fromY, int toX, int toY, char promoChar)
{
    int i, width = BOARD_RGHT - BOARD_LEFT;
    int to = toX - BOARD_LEFT + toY * width;
    int from = fromX - BOARD_LEFT + fromY * width;
    for(i=0; promote_pieces[i]; i++) if(promote_pieces[i] == promoChar) break;
    if(!promote_pieces[i]) i = 0;
    else if(i == 9 && gameInfo.variant == VariantChu) i = 1; // on 12x12 only 3 promotion codes available, so use 1 to indicate promotion
    if(fromY == DROP_RANK) i = 9, from = ToUpper(PieceToChar(fromX)) - '@';
    if(killX >= 0) { // multi-leg move
	int dx = killX - fromX, dy = killY - fromY;
	for(i=0; i<8; i++) if(dx == xStep[i] && dy == yStep[i]) {
	    int j;
	    dx = toX - killX; dy = toY - killY;
	    for(j=0; j<8; j++) if(dx == xStep[j] && dy == yStep[j]) {
		// special encoding in to-square, with promoType = 2. Assumes board >= 64 squares!
		return i + 8*j + (2 * width * BOARD_HEIGHT + from) * width * BOARD_HEIGHT;
	    }
	}
	i = 0; // if not a valid Lion move, ignore kill-square and promoChar
    }
    return to + (i * width * BOARD_HEIGHT + from) * width * BOARD_HEIGHT;
}

int
TextToMoves (char *text, int moveNum, entry_t *entries)
{
	int i, w, count=0;
	uint64 hashKey = hash(moveNum);
	int  fromX, fromY, toX, toY;
	ChessMove  moveType;
	char promoChar, valid;
	float dummy;

	entries[0].key = hashKey; // make sure key is returned even if no moves
	while((i=sscanf(text, "%f%%%d", &dummy, &w))==2 || (i=sscanf(text, "%d", &w))==1) {
	    if(i == 2) text = strchr(text, '%') + 1;  // skip percentage
	    if(w == 1) text = strstr(text, "1 ") + 2; // skip weight that could be recognized as move number one
	    valid = ParseOneMove(text, moveNum, &moveType, &fromX, &fromY, &toX, &toY, &promoChar);
	    text = strstr(text, yy_textstr) + strlen(yy_textstr); // skip what we parsed
	    if(!valid || moveType != NormalMove && moveType != WhiteDrop && moveType != BlackDrop
						&& moveType != FirstLeg
                                                && moveType != WhitePromotion && moveType != BlackPromotion
                                                && moveType != WhiteCapturesEnPassant && moveType != BlackCapturesEnPassant
                                                && moveType != WhiteKingSideCastle && moveType != BlackKingSideCastle
                                                && moveType != WhiteQueenSideCastle && moveType != BlackQueenSideCastle
                                                && moveType != WhiteNonPromotion && moveType != BlackNonPromotion) continue;
	    if(*text == ' ' && sscanf(text+1, "{%hd/%hd}", &entries[count].learnPoints, &entries[count].learnCount) == 2) {
		text = strchr(text+1, '}') + 1;
	    } else {
		entries[count].learnPoints = 0;
		entries[count].learnCount  = 0;
	    }
	    entries[count].move = CoordsToMove(fromX, fromY, toX, toY, promoChar); killX = killY = -1;
	    entries[count].key  = hashKey;
	    entries[count].weight = w;
	    count++;
	}
	return count;
}

Boolean bookUp;
int currentCount;

Boolean
DisplayBook (int moveNr)
{
    entry_t entries[MOVE_BUF];
    int count;
    char *p;
    if(!bookUp) return FALSE;
    count = currentCount = ReadFromBookFile(moveNr, appData.polyglotBook, entries);
    if(count < 0) return FALSE;
    p = MovesToText(count, entries);
    EditTagsPopUp(p, NULL);
    free(p);
    addToBookFlag = FALSE;
    return TRUE;
}

void
EditBookEvent()
{
      bookUp = TRUE;
	bookUp = DisplayBook(currentMove);
}

void
int_to_file (FILE *f, int l, uint64 r)
{
    int i;
    for(i=l-1;i>=0;i--) fputc(r>>8*i & 255, f);
}

void
entry_to_file (FILE *f, entry_t *entry)
{
    int_to_file(f,8,entry->key);
    int_to_file(f,2,entry->move);
    int_to_file(f,2,entry->weight);
    int_to_file(f,2,entry->learnCount);
    int_to_file(f,2,entry->learnPoints);
}

char buf1[4096], buf2[4096];

void
SaveToBook (char *text)
{
    entry_t entries[MOVE_BUF], entry;
    int count = TextToMoves(text, currentMove, entries);
    int offset, i, len1=0, len2, readpos=0, writepos=0;
    FILE *f;
    if(!count && !currentCount) return;
    f=fopen(appData.polyglotBook, "rb+");
    if(!f){	DisplayError(_("Polyglot book not valid"), 0); return; }
    offset=find_key(f, entries[0].key, &entry);
    if(entries[0].key != entry.key && currentCount) {
          DisplayError(_("Hash keys are different"), 0);
	  fclose(f);
	  return;
    }
    if(count != currentCount) {
	readpos = 16*(offset + currentCount);
	writepos = 16*(offset + count);
	fsseek(f, readpos, SEEK_SET);
	readpos += len1 = fread(buf1, 1, 4096 - 16*currentCount, f); // salvage some entries immediately behind change
    }
    fsseek(f, 16*(offset), SEEK_SET);
    for(i=0; i<count; i++) entry_to_file(f, entries + i); // save the change
    if(count != currentCount) {
	do {
	    for(i=0; i<len1; i++) buf2[i] = buf1[i]; len2 = len1;
	    if(readpos > writepos) {
		fsseek(f, readpos, SEEK_SET);
		readpos += len1 = fread(buf1, 1, 4096, f);
	    } else len1 = 0; // wrote already past old EOF
	    fsseek(f, writepos, SEEK_SET);
	    fwrite(buf2, 1, len2, f);
	    writepos += len2;
	} while(len1);
    }
    dirty = 1;
    fclose(f);
}

void
NewEntry (entry_t *e, uint64 key, int move, int result)
{
    e->key = key;
    e->move = move;
    e->learnPoints = 0;
    e->learnCount = 0;
    CountMove(e, result);
}

void
Merge ()
{
    int i;

    if(appData.debugMode) fprintf(debugFP, "book merge %d moves (old size %d)\n", mergeSize, bookSize);

    bookSize += --mergeSize;
    for(i=bookSize-1; mergeSize; i--) {
	while(mergeSize && (i < mergeSize || mergeBuf[mergeSize-1].key >= memBook[i-mergeSize].key))
	    memBook[i--] = mergeBuf[--mergeSize];
	if(i < 0) break;
	memBook[i] = memBook[i-mergeSize];
    }
    if(mergeSize) DisplayFatalError("merge error", 0, 0); // impossible
    mergeSize = 1;
    mergeBuf[0].key = -1LL;
}

void
AddToBook (int moveNr, int result)
{
    entry_t entry;
    int offset, start, move;
    uint64 key;
    int i, j, fromY, toY;
    char fromX, toX, promo;
extern char moveList[][MOVE_LEN];

    if(!moveList[moveNr][0] || moveList[moveNr][0] == '\n') return; // could be terminal position

    if(appData.debugMode) fprintf(debugFP, "add move %d to book %s", moveNr, moveList[moveNr]);

    // calculate key and book representation of move
    key = hash(moveNr);
    if(moveList[moveNr][1] == '@') {
	sscanf(moveList[moveNr], "%c@%c%d", &promo, &toX, &toY);
	fromX = CharToPiece(WhiteOnMove(moveNr) ? ToUpper(promo) : ToLower(promo));
	fromY = DROP_RANK; promo = NULLCHAR;
    } else sscanf(moveList[moveNr], "%c%d%c%d%c", &fromX, &fromY, &toX, &toY, &promo), fromX -= AAA, fromY -= ONE - '0';
    move = CoordsToMove(fromX, fromY, toX-AAA, toY-ONE+'0', promo);

    // if move already in book, just add count
    memBuf = (unsigned char*) memBook; bufSize = bookSize;   // in MC mode book resides in memory
    offset = find_key(NULL, key, &entry);
    while(memBook[offset].key == key) {
	if(memBook[offset].move == move) {
	    CountMove(memBook+offset, result); return;
	} else offset++;
    }
    // move did not occur in the main book
    memBuf = (unsigned char*) mergeBuf; bufSize = mergeSize; // it could be amongst moves still waiting to be merged
    start = offset = find_key(NULL, key, &entry);
    while(mergeBuf[offset].key == key) {
	if(mergeBuf[offset].move == move) {
            if(appData.debugMode) fprintf(debugFP, "found in book merge buf @ %d\n", offset);
	    CountMove(mergeBuf+offset, result); return;
	} else offset++;
    }
    if(start != offset) { // position was in mergeBuf, but move is new
        if(appData.debugMode) fprintf(debugFP, "add in book merge buf @ %d\n", offset);
	for(i=mergeSize++; i>offset; i--) mergeBuf[i] = mergeBuf[i-1]; // make room
	NewEntry(mergeBuf+offset, key, move, result);
	return;
    }
    // position was not in mergeBuf; look in hash table
    i = (key & mask); offset = -1;
    while(hashTab[i].key) { // search in hash table (necessary because sought item could be re-hashed)
	if(hashTab[i].key == 1 && offset < 0) offset = i; // remember first invalidated entry we pass
	if(!((hashTab[i].key - key) & ~1)) { // hit
	    if(hashTab[i].move == move) {
		CountMove(hashTab+i, result);
		for(j=mergeSize++; j>start; j--) mergeBuf[j] = mergeBuf[j-1];
	    } else {
		// position already in hash now occurs with different move; move both moves to mergeBuf
		for(j=mergeSize+1; j>start+1; j--) mergeBuf[j] = mergeBuf[j-2];
		NewEntry(mergeBuf+start+1, key, move, result); mergeSize += 2;
	    }
	    hashTab[i].key = 1; // kludge to invalidate hash entry
	    mergeBuf[start] = hashTab[i]; mergeBuf[start].key = key;
	    if(mergeSize >= MERGESIZE) Merge();
	    return;
 	}
	i = i+1 & mask; // wrap!
    }
    // position did not yet occur in hash table. Put it there
    if(offset < 0) offset = i;
    NewEntry(hashTab+offset, key, move, result);
    if(appData.debugMode)
	fprintf(debugFP, "book hash @ %d (%d-%d)\n", offset, hashTab[offset].learnPoints, hashTab[offset].learnCount);
}

void
AddGameToBook (int always)
{
    int i, result;

    if(!mcMode && !always) return;

    InitMemBook();
    switch(gameInfo.result) {
      case GameIsDrawn: result = 1; break;
      case WhiteWins:   result = 2; break;
      case BlackWins:   result = 0; break;
      default: return; // don't treat games with unknown result
    }

    if(appData.debugMode) fprintf(debugFP, "add game to book (%d-%d)\n", backwardMostMove, forwardMostMove);
    for(i=backwardMostMove; i<forwardMostMove && i < 2*appData.bookDepth; i++)
	AddToBook(i, WhiteOnMove(i) ? result : 2-result); // flip result when black moves
}

void
PlayBookMove(char *text, int index)
{
    char *start = text+index, *end = start;
    while(start > text && start[-1] != ' ' && start[-1] != '\t') start--;
    while(*end && *++end != ' ' && *end != '\n'); *end = NULLCHAR; // find clicked word
    if(start != end) TypeInDoneEvent(start); // fake it was typed in move type-in
}

void
FlushBook ()
{
    FILE *f;
    int i;

    InitMemBook();
    Merge(); // flush merge buffer to memBook

    if(f = fopen(appData.polyglotBook, "wb")) {
	for(i=0; i<bookSize; i++) {
	    entry_t entry = memBook[i];
	    entry.weight = entry.learnPoints;
//	    entry.learnPoints = 0;
//	    entry.learnCount  = 0;
	    entry_to_file(f, &entry);
	}
    } else DisplayError(_("Could not create book"), 0);
}
