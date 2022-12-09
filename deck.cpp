#include "deck.h"

namespace tinybot {

Offer::Offer(const int t0, const int t1, const int z0, const int z1) {
  offer_tile[0] = t0;
  offer_tile[1] = t1;
  offer_zone[0] = z0;
  offer_zone[1] = z1;
}

Offer::Offer(const std::string& s) {
  offer_tile[0] = parseTile(s[0]);
  offer_tile[1] = parseTile(s[2]);
  offer_zone[0] = parseZone(s[1]);
  offer_zone[1] = parseZone(s[3]);
}

void PcgDeck::offer(Board* board) {
  const int n = board->deck[0];
  board->deck[0] -= 2;
  int c = rng(n);
  int d = rng(n);
  int e = rng(n - 1);
  int f = rng(n - 1);
  for (int i = kForest; i <= kBoat; ++i) {
    c -= board->deck[i];
    if (c < 0) {
      --board->deck[i];
      board->offer_tile[0] = i;
      break;
    }
  }
  for (int i = 0; i < 27; ++i) {
    d -= board->zone[i];
    if (d < 0) {
      --board->zone[i];
      board->offer_zone[0] = i;
      break;
    }
  }
  for (int i = kForest; i <= kBoat; ++i) {
    e -= board->deck[i];
    if (e < 0) {
      --board->deck[i];
      board->offer_tile[1] = i;
      break;
    }
  }
  for (int i = 0; i < 27; ++i) {
    f -= board->zone[i];
    if (f < 0) {
      --board->zone[i];
      board->offer_zone[1] = i;
      break;
    }
  }
}

FixedDeck::FixedDeck(const std::string& s) {
  init(split(s));
}

FixedDeck::FixedDeck(const std::vector<std::string>& tokens) {
  init(tokens);
}

void FixedDeck::offer(Board* board) {
  Offer my_offer = offers.front();
  board->deck[0] -= 2;
  --board->deck[my_offer.offer_tile[0]];
  --board->deck[my_offer.offer_tile[1]];
  --board->zone[my_offer.offer_zone[0]];
  --board->zone[my_offer.offer_zone[1]];
  board->offer_tile[0] = my_offer.offer_tile[0];
  board->offer_tile[1] = my_offer.offer_tile[1];
  board->offer_zone[0] = my_offer.offer_zone[0];
  board->offer_zone[1] = my_offer.offer_zone[1];
  offers.pop();
}

void FixedDeck::init(const std::vector<std::string>& tokens) {
  for (int i = 0; i <= 16; i += 2) {
    if (i >= tokens.size()) {
      return;
    }
    offers.emplace(tokens[i]);
  }
  for (int i = 19; i <= 35; i += 2) {
    if (i >= tokens.size()) {
      return;
    }
    offers.emplace(tokens[i]);
  }
  for (int i = 38; i <= 52; i += 2) {
    if (i >= tokens.size()) {
      return;
    }
    offers.emplace(tokens[i]);
  }
}

FixedDeck* kFixedShuffles[kNumFixedShuffles] {
  new FixedDeck("CWfC . w1sd . M2MA . waCX . w3fc . wQff . MDw6 . h2Cc . fWh7 . . hSfE . f8hg . BXB7 . f9wa . fQwS . w9B5 . hbCZ . fbfC . fhs3 . . CDh8 . Mihh . s4sZ . s4ff . sEs5 . fdwi . h6he . sghA . #20201223"),
  new FixedDeck("CWfC . w1sd . M2MA . waCX . w3fc . wQff . MDw6 . h2Cc . fWh7 . . hSfE . f8hg . BXB7 . f9wa . fQwS . w9B5 . hbCZ . fbfC . fhs3 . . CDh8 . Mihh . s4sZ . s4ff . sEs5 . fdwi . h6he . sghA . #20201223"),
  new FixedDeck("CWfC . w1sd . M2MA . waCX . w3fc . wQff . MDw6 . h2Cc . fWh7 . . hSfE . f8hg . BXB7 . f9wa . fQwS . w9B5 . hbCZ . fbfC . fhs3 . . CDh8 . Mihh . s4sZ . s4ff . sEs5 . fdwi . h6he . sghA . #20201223"),
  new FixedDeck("s2wh . fAMS . hSB4 . C8h9 . fCsE . ffsi . CXh1 . f7hZ . fDfe . . sgf9 . w4wd . McBe . sawZ . MWhb . w2Cb . fQsc . CQh8 . w1BA . . w3ha . fDMW . M3sX . fCwE . h5f5 . s7f6 . hdC6 . shhi . #slogo0"),
  new FixedDeck("M9w8 . f4sc . hgM1 . hWwX . fAhb . Cffg . s1fQ . fdhX . fDwE . . wdfe . hSs5 . C7fW . sCw6 . CEhD . hcBZ . C8BZ . w7sa . M5fi . . waC3 . f2hi . wSwb . sQf3 . sAfh . h4M9 . feh2 . s6Mh . #slogo1"),
  new FixedDeck("hDf4 . h7f6 . f8wD . M3sb . f4C1 . hXh1 . sgsE . w8CW . h3Ci . . few7 . w5he . f2fW . fgsd . hQfA . Cbw9 . sCMQ . fEfa . whMS . . f5sX . M6wd . C2Bc . Bcha . M9fZ . sCBf . hihZ . whsA . #20190713"),
  new FixedDeck("McMC . s2wa . hAhe . f8sW . hQfi . wAMd . s2CX . hdfC . Cesh . . f7w4 . w6fc . B3wf . wZhQ . C5wE . h1sS . sgff . s1sg . fWf8 . . BXf9 . BahD . w9hZ . hDM7 . fbf4 . f5hS . w3fh . sbC6 . #20190829"),
  new FixedDeck("ffCW . w9fc . wDhh . f5Bh . s6f9 . MdM6 . hasX . MEwW . f2wb . . M5hc . h4sA . wZh4 . fQhb . Mfw7 . hCwe . s8fS . C8f3 . Bifd . . hZsX . seCE . B1wC . saf3 . fisD . fSCA . wQh7 . C2fg . #20190904"),
  new FixedDeck("hiw6 . fhhQ . hQCA . fZwc . fgCS . w5w7 . feCb . sDf1 . f4w6 . . sdh2 . B1fg . sZhX . h8sE . BbfE . wah3 . w5he . C3sW . wCMf . . fcf4 . MSs9 . BXsW . h9sA . f8Md . MfC2 . hCMi . f7wa . #20191020"),
  new FixedDeck("C5BX . wCB9 . fbCZ . h5si . Mgfi . fCwg . fDfc . w6wf . hDwe . . sbC3 . fWs1 . hSff . hZsQ . wEME . sQha . BdwA . safS . h4wd . . hhC2 . fef7 . hhM6 . f7M2 . C3fX . Mch9 . h4s8 . sWs1 . #20201001"),
  new FixedDeck("Mbs6 . h4s4 . Mgwf . CCwQ . fchc . M3fZ . h2fg . B7h6 . fas8 . . ffsW . BDfi . f9fh . fSfX . h5h9 . weCW . hDf1 . Bhsd . wZwA . . h3w5 . CEse . fiwE . wQCS . h2MC . sAsX . M1C8 . has7 . #20201002"),
  new FixedDeck("C5w6 . M7h5 . f2Bb . hgCZ . wgsi . f2wf . M9s3 . f4hd . sdfe . . wEfS . wbsf . hDf4 . sAw3 . fWfQ . shhQ . hiM7 . hEhC . hhM8 . . sZha . s8MW . wAfX . wcC6 . weBc . fSfa . CDCX . s1BC . #20201003"),
  new FixedDeck("f5hS . s3sh . CiMf . wcf2 . fcw6 . fXw2 . CSse . s7f8 . hWM6 . . wAfX . h3h9 . hEfe . sbMh . fAf4 . sdsf . f4CE . hDh1 . sgf8 . . Bdf5 . saBQ . C1M9 . hiMa . w7fC . wbwg . hQCD . BWhC . #20201004"),
  new FixedDeck("hifD . f7fg . hhf9 . MesQ . h4hf . BAM8 . w9BX . MQwa . Cah3 . . B1fg . wZsd . wefi . hSs6 . sbwW . s1f2 . h4sD . fhfA . Mfh3 . . wCfZ . s5MX . wEh2 . scfE . CdfS . wcC5 . CbsC . f7h8 . #20201005"),
  new FixedDeck("h1wg . hZsS . wbhd . f2w8 . h6wQ . fAsb . hSf8 . wehQ . BDM7 . . fawC . B5s5 . s9Cf . fEfC . hDs3 . fEs4 . f6MA . MXfa . C2Cf . . f1Cc . sgw7 . MWs3 . MXCi . w4hi . fefh . B9fd . hZwh . #20201006"),
  new FixedDeck("hWB5 . hhM9 . wZM4 . fDha . w5Bb . BSfa . fZf6 . sSsX . wAMW . . h3CX . fAC7 . s3M1 . hds2 . hEw1 . Cew2 . wCsc . feMi . fDhC . . hhfg . f8CQ . f6hc . wgs7 . sbff . w8sd . sQf4 . CEff . #20201007"),
  new FixedDeck("saw5 . wAhD . fcf8 . CSf6 . fcsh . wdsE . w3fW . hbBQ . wbfa . . hes5 . MWMf . hXC2 . fgh7 . s9hZ . fish . BDs8 . hZM4 . weM4 . . fSCC . h3Ci . h7w1 . BAC2 . s9sC . f1fd . wgh6 . fQfE . #20201008"),
  new FixedDeck("scCd . wff1 . w2sD . wSBd . scsa . f4fW . fXhg . s6M8 . hACX . . sEMi . hEh2 . f7hW . B6MZ . CZsa . B7w8 . w9w5 . hewb . fef3 . . MSfQ . s4f5 . Cifh . fCwf . w9sD . MQh1 . hChg . fAhb . #20201009"),
  new FixedDeck("hZfh . hAf3 . wWwh . w2s3 . M5BW . sCsZ . wAf8 . fQfd . MDCQ . . fXf7 . h9wg . sEfc . wdsf . fSsi . MSff . h2hX . fbwe . B6hb . . Cahg . fcs8 . B5f4 . s9C6 . s1M1 . hEC4 . M7Ci . wawC . #20201010"),
  new FixedDeck("fECX . fgwi . fQBQ . Mfh9 . f2w8 . hdCc . s7h7 . CWhg . f1Cb . . f9MC . hhf4 . fWMX . C6hC . wcMe . hSsa . fhw1 . f3wD . wSh3 . . wbsZ . s2hA . s8wA . s4he . BdwE . fas5 . s5Bi . fffZ . #20201011"),
  new FixedDeck("fQh8 . fSfe . hDhh . w7hE . s2fg . CdMX . s7C2 . CAhC . s1fW . . M8C5 . fawi . f3fb . Bds9 . wahW . s3wZ . M1f5 . BXh6 . w9BA . . sSfZ . fEsf . h6Cf . whwc . McfD . MCwg . s4fe . hisQ . #20201012"),
  new FixedDeck("fQC5 . C6sC . hisW . w8ff . fDMd . w5s2 . weC8 . hbfc . wZMf . . MSh4 . hCwg . fAs3 . sgM4 . B1wa . fZhc . hQB7 . Cbw9 . h7h2 . . hefE . fhsS . f9si . fXsE . fDBd . C6fa . h1s3 . MWfA . #20201013"),
  new FixedDeck("s8BE . hWC9 . wEs7 . s3h4 . h1hW . BgCb . MZsc . faff . fSsQ . . sXwA . fdwe . ffhb . hhw8 . Mefc . wCf5 . h2f6 . fZhg . Cdf7 . . w1BD . f5Ma . M9s4 . wXhi . MSwh . h2fQ . wAf6 . CDC3 . #20201014"),
  new FixedDeck("M6fQ . f2M9 . h3wa . hAhe . fesE . fXB8 . wgwZ . C2hD . hifg . . C8M5 . fcC4 . wfB1 . fcsa . shs1 . fAwE . BSfS . s9hQ . sdhh . . MWsW . fXs4 . hDCi . MZwC . CCh5 . w7wb . f3sb . f6hf . #20201015"),
  new FixedDeck("fafc . w9s5 . sQBd . fEh9 . f6sE . h3hA . w7CW . BcC1 . safi . . w1w6 . fehb . sffh . CffQ . s7sg . f8f4 . wXfS . MDhd . f2Bg . . MhsZ . h4wi . Mbf3 . wXhD . hSwe . hZM2 . CAhC . wCs8 . #20201016"),
  new FixedDeck("sAhS . wEME . fesX . sXwi . h4Bb . M7MZ . MawC . M4fQ . w2h3 . . h3f8 . wZCg . s2wC . BfBc . f9ha . s6hh . fdhW . ffsb . C5f8 . . h7hD . CghA . w1se . fDf1 . fQsh . fdwW . CSC5 . fifc . #20201017"),
  new FixedDeck("s8fg . wih6 . f5ME . s5f4 . wAsg . fDsE . h6wf . Bhfc . wafS . . C3wD . wCh2 . hbBW . w8f9 . CQBh . s7MZ . w1MC . heCa . hZfc . . wes3 . hSs4 . h7fd . fbsf . MXCW . fXCi . fdh1 . MQfA . #20201018"),
  new FixedDeck("fCMe . MQhX . w4wb . M5fS . wasg . fehA . h6sg . C7ff . s7wD . . Bdsc . f9w8 . f5f3 . s9ff . Cihd . f1wh . s3BD . hEsa . wWs1 . . sbMW . C4hC . fcMZ . wXh2 . fQh8 . B6hE . fZfA . CSwh . #20201019"),
  new FixedDeck("hCMg . wAwa . hShe . C7he . sifg . s8f5 . Cih3 . fch8 . fQhE . . s9fW . w4sX . s2Bd . MEB9 . fXha . w6fZ . C3wh . B7wc . hZwD . . sbfQ . wAsD . sbwW . f1M5 . fdf4 . MSsf . M2ff . CCh6 . #20201020"),
  new FixedDeck("fhBa . f9f9 . CDCf . wcs1 . wefA . MQsg . hXMf . hAwa . h7hD . . sbCZ . wEfi . MbhS . f4f8 . shh6 . sWfE . hdsQ . sgM4 . C8M2 . . s7hC . wifW . f5w3 . wdwc . C2s3 . hCf1 . Bef6 . fZwX . #20201021"),
  new FixedDeck("hSw1 . BEfQ . C9Md . sihX . sABg . ffwZ . hDC5 . wWf7 . w3h9 . . fefA . M6hS . f2wb . sfhc . fif8 . fQs5 . sXfW . C8h3 . s1Mh . . h4w4 . fZf2 . Bgsh . MahE . haM7 . sewC . CdCD . wCfc . #20221112"),
  new FixedDeck("fas4 . fhf8 . w6h9 . sShW . CEfE . hih5 . s8Bg . Cff9 . hcfS . . sZfd . fif3 . sQw6 . wWh5 . f3hh . wfsb . wch4 . wQCe . f2BC . . MgMZ . sXfC . wAsX . hbCd . w7h1 . BewA . CafD . M7sD . #20221113"),
  new FixedDeck("wXC7 . fbsC . shhE . CisE . hiff . fWf5 . sghD . hWsZ . Cew4 . . h1MD . f7w2 . MSBe . wXwa . f2s5 . ffwA . h3sQ . fAMd . h9w4 . . wgsc . f3f8 . f6f6 . wdBQ . BSs9 . MZh1 . CChb . Mchh . #20221114"),
  new FixedDeck("hAB9 . wWsa . f2wf . hhMe . sef2 . fih3 . w6fZ . CSsX . fis1 . . wQwW . f3hD . sDhg . wEhb . hcCh . MEs7 . waCd . hQfc . f8fg . . M5BS . fCfZ . s6Bd . C8w9 . MCf5 . hXhf . CAM7 . w1s4 . #20221115"),
  new FixedDeck("M6h3 . fahW . sDfh . fAsh . MQwW . sXs2 . wff1 . wDsf . Mes8 . . sCCE . hdf7 . M8C6 . C2fC . hifg . BAh5 . MafE . h4s9 . f7C4 . . CSw1 . wcf5 . w3wd . fShc . hXBe . BbwZ . s9hb . hiwg . #20221116"),
  new FixedDeck("wgs6 . MSMi . wafX . fbCQ . Cfhc . f1f7 . wXfh . C3B9 . MCB9 . . h2wa . wZsc . Mehf . wChg . s2Ce . f8h7 . fDhE . s5hA . fEhh . . C6wb . his5 . M8s1 . f4hW . f3fW . sdsS . wdB4 . wDsZ . #20221117"),
  new FixedDeck("MQsg . sCh1 . wWM7 . sbh2 . MAf8 . fXhD . hdsW . CZCi . s7hC . . sfsA . hdM8 . wEB4 . f2wZ . ffw3 . wgs5 . fSfE . s3hh . C4wb . . wchQ . wefe . McB6 . hawa . fhfD . f6f9 . CXB5 . f1C9 . #20221118"),
  new FixedDeck("BXBd . hZfa . sAB3 . wWw9 . C5fQ . s7Mh . wiC2 . wSfC . Meh6 . . sCwh . hEfD . MbC9 . sasc . h8Md . hgC4 . h7wi . sZsc . Mff4 . . fXfg . wQCD . fbs1 . h5hA . fes8 . hff1 . hWfS . wEw6 . #20221119"),
  new FixedDeck("hbhi . sACW . h8h1 . fefh . hSfd . sZf3 . w1MD . sDh2 . fWfg . . s5sQ . hfsd . MEB4 . MXf9 . C4w6 . wCMQ . f2ha . hfs7 . fbhE . . f6w8 . wgCe . scCc . wAfi . w3f9 . B5fX . BaCh . wZM7 . #20221120"),
  new FixedDeck("h9w9 . fEB2 . fgsa . Mhhi . f2sg . fQh4 . s6w7 . f1Cb . s5Mi . . sdBa . ffhZ . sXCA . hAh1 . fWC8 . sSMC . s8h4 . hEfe . shwd . . fWC3 . C3hc . w7MZ . fQMX . h6wC . wbwe . wDf5 . wSfD . #20221121"),
  new FixedDeck("sgfC . h8CS . wehc . seB7 . hAfa . f8w6 . CdfD . wiM3 . f3wC . . MQw2 . waf5 . sbM9 . Mds4 . CXB1 . f7h9 . hfsh . wghf . hQsZ . . fEfE . h5fX . fWCD . fhw1 . s6hc . sWsb . MiB4 . w2hZ . #20221122"),
  new FixedDeck("C9M1 . scs4 . h8s4 . h7sW . whwf . sDC2 . Mdf7 . f2fh . sdCD . . Bifi . f9MQ . BgwX . hZsS . hbfE . CEh3 . fCMZ . ffhC . h6fA . . fghX . h5CW . fes1 . wSwA . MeBb . wawa . f5w3 . h6wc . #20221123"),
  new FixedDeck("CWM7 . f1fa . Mff6 . w2sD . f3hd . h3fA . h7fd . hhsh . wbsA . . sDCW . sEhC . C4fZ . sch9 . f8we . wXM2 . MeBC . MXw9 . s4f5 . . wiwQ . fchQ . BSsg . fZB1 . wgfa . wSh5 . hbsE . f8Cf . #20221124"),
  new FixedDeck("w5fD . fQhd . h3hA . f7B8 . sSMZ . ffhC . h9Cb . wWM1 . M3fX . . B4fh . Casg . sCh8 . wAfg . CDsX . fiBW . s6h9 . sEse . s4f1 . . wdwf . wcwe . sSC2 . Mif7 . faw6 . CQMb . hcw2 . hEh5 . #20221125"),
  new FixedDeck("h1wE . fgCe . hSf1 . Bbfd . hah9 . M8hA . fah7 . hhMh . BEfX . . s9hX . fWBe . fDwZ . sQsD . sdhA . w4MW . fQf2 . M3C6 . sCfi . . Miwf . f4wS . h6w5 . fZCc . s5sC . wfC8 . w2C7 . w3fc . #20221126"),
  new FixedDeck("hAf3 . fhwg . BXh3 . w9Ce . MShi . f4MZ . sQfa . wiw6 . fDf2 . . s6h9 . CAB8 . s7fa . CEC5 . sCfc . sffS . wWhd . Mes7 . sCM8 . . wdCX . hQB1 . fghZ . sDwW . h4fc . MEwh . s2h5 . f1wb . #20221127"),
  new FixedDeck("ffhW . fShe . sXwE . f1hc . M6C7 . f4f1 . hhff . fgBD . C3f9 . . fchg . w9CZ . fbBC . s6fZ . wAh5 . w5Cb . C2M7 . sisX . MEwW . . s2hD . wdM8 . sSMa . w4fC . s3sQ . f8wi . wdBQ . heha . #20221128"),
  new FixedDeck("sbw2 . fAfQ . w6hg . M3fW . fCwX . hSfb . sZfd . h4Mf . hEC4 . . fhf1 . M5s9 . saC1 . hEfi . B8w3 . C9w5 . sCfe . fisQ . wDBf . . hcwZ . sewD . CdhS . h7M7 . hAs2 . CgM8 . fcfX . B6sh . #20221129"),
  new FixedDeck("Bhh7 . sZwb . wfsD . Cew4 . s7Mb . fEsa . CXMd . fiB2 . h3w9 . . hZs3 . fgf1 . h9wd . BAMQ . haw4 . w8wc . h5CS . fWsC . fDs8 . . fgCW . f6sc . fffA . MSs6 . hXfi . feMQ . C5hC . f1wE . #20221130"),
  new FixedDeck("f5w1 . CgMc . BcCX . h9s1 . wbfW . wesS . hbw4 . f7C2 . fDf7 . . hQf5 . sgB3 . fAMa . hXM4 . heMC . shha . ffME . Cff2 . hih6 . . wAhZ . sDfW . wEs9 . f3s8 . w6fi . BQwh . sZhS . fdC8 . #20221201"),
  new FixedDeck("sdsS . fAh4 . f6w7 . fhs2 . sQhW . w1fg . h5f5 . BAs3 . fZwf . . M6Mb . MahX . hCfe . MWsD . f2hd . wfBE . h3wa . sZsQ . w8CE . . wewc . CcCD . fChi . f9Ch . fSC1 . hXM7 . B8s4 . f9hi . #20221202"),
  new FixedDeck("fbhi . CDfW . CCCX . h7f1 . wgf9 . hSsh . wesA . hefE . BEsS . . f9h1 . wgwh . MDfb . s4hd . wifZ . s2h5 . wWh8 . f3B7 . MQsZ . . hXC8 . BAwc . Maw5 . MCfc . f2s6 . s3Md . w4sa . fQff . #20221203"),
  new FixedDeck("sZCg . fWs1 . s9w1 . wdhD . sSfA . MfC6 . fCw7 . h6sQ . fWhi . . fDC8 . h3Mc . whf4 . s3Cf . s8Bi . sEf9 . MAwX . M2M5 . hawX . . hbfe . hehS . CEh7 . fCsZ . fgB4 . fQfa . w5fh . wcwd . #20221204"),
  new FixedDeck("w2wc . f3sb . Bhff . s9hg . hEBW . sgfZ . wdhQ . CDM8 . f1fX . . f9hd . s5w7 . B1wS . MaCQ . fChc . fDfA . s3hf . hWCZ . MeCi . . f4h7 . hSfb . hEs6 . whsi . f2wA . sXfe . s8M6 . C4wa . #20221205"),
  new FixedDeck("sEsA . fWh1 . wXhg . Cche . M7wb . MCM2 . w2fX . fDwd . fDfb . . s5f6 . f8ff . hhw9 . whCe . hShC . h7sA . CZhE . B1f8 . saCS . . B4wc . fZw3 . M6f5 . C4wg . BihQ . f9sW . sQfi . Mfs3 . #20221206"),
  new FixedDeck("hXfc . fhwX . C9Cb . w1sC . BDf1 . sZfD . siB6 . MEha . fChW . . BSMe . h9sa . C7f8 . fiC6 . h5f7 . wAsd . wewW . hSw5 . fgs3 . . wQh4 . sQs8 . fdhf . hffb . CEM3 . fcs2 . wghA . M2fZ . #20221207"),
  new FixedDeck("f7wS . C5fQ . his6 . wACW . M3Cc . was1 . sbh1 . f4fb . C6wD . . hCM2 . fhs2 . BdhA . fZwW . fhfc . hQw3 . f7w5 . h9fS . sEwg . . hdsD . s8f8 . wiBX . fes9 . hfhf . heBZ . MgMa . MXsC . #20221208"),
  new FixedDeck("C3sf . M1Ci . C2M7 . fih4 . h6Mg . hewX . CZhe . fCfW . shhD . . sAfh . w8wS . hQBb . MAwZ . sCf2 . f4BX . C6f9 . scff . hWw9 . . fEhS . wghb . M5B8 . fawa . sQfD . wEf3 . scs5 . fdh1 . #20221209"),
  new FixedDeck("CZsA . Mcwd . fXh5 . sDfW . h1hS . C8MA . w1M4 . hcw2 . ChfQ . . sCCh . fWBi . w5f9 . sDwQ . f4f2 . sfMX . w8wC . feBS . wgsa . . C3Bi . h7fZ . sEh6 . Mfsg . fahE . hdfe . w7f9 . fbh6 . #20221210"),
  new FixedDeck("hSw1 . BEfQ . C9Md . sihX . sABg . ffwZ . hDC5 . wWf7 . w3h9 . . fefA . M6hS . f2wb . sfhc . fif8 . fQs5 . sXfW . C8h3 . s1Mh . . h4w4 . fZf2 . Bgsh . MahE . haM7 . sewC . CdCD . wCfc . #20221211"),
  new FixedDeck("fEM6 . f6f4 . Chf9 . BCfD . fZsX . hSf4 . MWh5 . haw7 . h8MS . . sABX . his9 . hgwd . wbMe . hcBC . hhsa . sZhf . wWwe . f3fb . . Cdsf . M1f8 . w2wQ . C2C5 . scw7 . s1fD . hQC3 . fEsi . #20221212"),
  new FixedDeck("BiM3 . f9fC . wXfg . Mefh . w4f3 . CSsD . fChE . h9w7 . wEsc . . BQhf . wZCW . hbh1 . wZwg . hDf5 . CeM5 . hcsX . shhA . s8s8 . . MdfW . C2BQ . fifa . C6fd . w7hS . s2hb . f4MA . f6sa . #20221213"),
  new FixedDeck("sEMD . hifa . hefQ . sgh6 . wdsW . CSs8 . MCB4 . C4f9 . M1ff . . s2C3 . hfwc . wZhD . fQs8 . BXwA . wZfc . M5f5 . sCfg . wWh9 . . fefh . M6CS . sXw7 . w1hb . C3h7 . hiBA . fbfa . shwd . #20221214"),
  new FixedDeck("M8wC . fahD . Mgw8 . fXCb . MXBi . f4f9 . w6hS . hcwW . fZs2 . . B6wf . sZf3 . MWhf . fCCA . wDs1 . h5Cd . h7M7 . fhs2 . fQhE . . hhsb . fes4 . h3wi . fgwe . C9fS . wcsd . sQB1 . fAha . #20221215")
};

}  // namespace tinybot
