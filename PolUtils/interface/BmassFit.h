#ifndef PHYSUTILS_POLUTILS_BMASSFIT_H__
#define PHYSUTILS_POLUTILS_BMASSFIT_H__

#include "general/root_utils.h"
#include "general/roofit_utilities.h"

#include "config/GeneralJpsiFromB.h"

#include "RooWorkspace.h"
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooAddPdf.h"
#include "RooArgList.h"
#include "RooFitResult.h"
#include "RooPlot.h"

#include "TCanvas.h"
#include "TH1D.h"
#include "TLegend.h"
#include "TString.h"
#include "TLatex.h"
#include "TLine.h"

#include <vector>
#include <string>
#include <utility>
#include <tuple>
#include <cmath>

void setupMassFit(RooWorkspace* ws, const FitModel& fitModel,
                  const std::vector<std::pair<std::string, double> >& constVals)
{
  fitModel.importToWorkspace(ws);

  // still have to build the signal pdf here, actually moved to the config file
  // TODO: write a wrapper that handles ws->var and ws->function within one call
  // RooAddPdf model("Bmass_model", "Bmass_model",
  //                 RooArgList(*ws->pdf("pdf_m_signal"), *ws->pdf("pdf_m_combinatorial"), *ws->pdf("pdf_m_jpsipi"),
  //                            *ws->pdf("pdf_m_nonprompt")),
  //                 RooArgList(*ws->var("n_signal"), *ws->var("n_combinatorial"), *ws->function("n_jpsipi"),
  //                            *ws->function("n_nonprompt"))
  //                 );

  // ws->import(model);

  ws->Print("v");
  setConstants(ws, constVals);
}

void doFit(RooWorkspace* ws, const std::string& modelName)
{
  using namespace RooFit;

  ws->var("n_signal")->setVal(ws->data("jpsi_fromB_data_rap0_pt0")->sumEntries("abs(Bmass - 5.27926) < 0.015"));
  ws->var("n_combinatorial")->setVal(ws->data("jpsi_fromB_data_rap0_pt0")->sumEntries("abs(Bmass -5.27926) > 0.015"));

  // ws->Print("v");

  std::cout << ws->pdf(modelName.c_str()) << std::endl;
  ws->pdf(modelName.c_str())->fitTo(*ws->data("jpsi_fromB_data_rap0_pt0"),
                                    Minos(false), NumCPU(4), Offset(false),
                                    Extended(true));

  RooFitResult* res_data = ws->pdf(modelName.c_str())->fitTo(*ws->data("jpsi_fromB_data_rap0_pt0"),
                                                             Minos(true), NumCPU(4), Offset(false), Save(true),
                                                             Extended(true));

  std::cout << "=============================================" << std::endl;
  std::cout << res_data->status() << " " << res_data->covQual() << std::endl;

  ws->import(*res_data);
}

TCanvas* setupCanvas()
{
  TCanvas* can = new TCanvas("c", "c", 800, 800);

  return can;
}


void frameDressig(RooPlot* frame, TString xtitle = "", TString ytitle = "")
{
  frame->SetTitle("");
  frame->GetXaxis()->SetTitle(xtitle);
  frame->GetXaxis()->SetLabelFont(42);
  frame->GetXaxis()->SetLabelOffset(0.01);
  frame->GetXaxis()->SetTitleSize(0.03);
  frame->GetXaxis()->SetTitleOffset(1.09);
  frame->GetXaxis()->SetLabelFont(42);
  frame->GetXaxis()->SetLabelSize(0.03);
  frame->GetXaxis()->SetTitleFont(42);
  frame->GetYaxis()->SetTitle(ytitle);
  frame->GetYaxis()->SetLabelFont(42);
  frame->GetYaxis()->SetLabelOffset(0.01);
  frame->GetYaxis()->SetTitleOffset(1.14);
  frame->GetYaxis()->SetTitleSize(0.03);
  frame->GetYaxis()->SetTitleFont(42);
  frame->GetYaxis()->SetLabelFont(42);
  frame->GetYaxis()->SetLabelSize(0.03);
}

TCanvas* plotMassProjection(RooWorkspace *ws, const std::tuple<double, double, double, double>& SRandFrac)
{
  using namespace RooFit;
  TCanvas *c1 = setupCanvas();

  const int nbins = 100;

  RooRealVar *mass = ws->var("Bmass");
  RooAbsData *data = ws->data("jpsi_fromB_data_rap0_pt0");
  RooAbsPdf *model = ws->pdf("Bmass_model");

  RooPlot* frame_m = mass->frame();

  TH1D* histo_data = (TH1D*)data->createHistogram("B_mass_data", *mass, Binning(nbins,config::Bplus.massMin,config::Bplus.massMax));
  histo_data->Sumw2(false);
  histo_data->SetBinErrorOption(TH1::kPoisson);
  histo_data->SetMarkerStyle(20);
  histo_data->SetMarkerSize(0.8);
  histo_data->SetLineColor(kBlack);
  for (int i=1; i<=nbins; i++)
    if (histo_data->GetBinContent(i)==0) histo_data->SetBinError(i,0.);

  data->plotOn(frame_m,Binning(nbins),Invisible());

  model->plotOn(frame_m,Name("Bmass_model"),Precision(2E-4));
  //model->plotOn(frame_m,Name("signal"),Precision(2E-4),Components("pdf_m_signal"),LineColor(kRed),LineWidth(2),LineStyle(kSolid),FillStyle(3008),FillColor(kRed), VLines(), DrawOption("F"));
  model->plotOn(frame_m,Name("signal"),Precision(2E-4),Components("pdf_m_signal"),LineColor(kRed-10),LineWidth(2),LineStyle(kSolid),FillColor(kRed-10), VLines(), DrawOption("F"));

  model->plotOn(frame_m,Name("combinatorial"),Precision(2E-4),Components("pdf_m_combinatorial"),LineColor(kCyan+2),LineWidth(3),LineStyle(7));

  if (ws->var("f_jpsipi")->getVal()>0.)
    model->plotOn(frame_m,Name("jpsipi"),Precision(2E-4),Components("pdf_m_jpsipi"),LineColor(kViolet),LineWidth(0),LineStyle(kSolid),FillStyle(3145),FillColor(kViolet), VLines(), DrawOption("F"));
  if (ws->var("f_nonprompt")->getVal()>0.)
    model->plotOn(frame_m,Name("nonprompt"),Precision(2E-4),Components("pdf_m_nonprompt"),LineColor(kOrange-6),LineWidth(3),LineStyle(3));

  frameDressig(frame_m, "M_{J/#psi K^{#pm}} [GeV]", TString::Format("Entries / %g MeV",(config::Bplus.massMax-config::Bplus.massMin)*1000./nbins));

  frame_m->SetMinimum(0.);
  frame_m->SetMaximum(frame_m->GetMaximum()*1.1);

  frame_m->Draw();
  histo_data->Draw("Esame");

  int nitems = 5;
  if (ws->var("f_jpsipi")->getVal()>0.) nitems++;
  if (ws->var("f_nonprompt")->getVal()>0.) nitems++;


  TLegend *leg = new TLegend(0.5,0.9-0.06*nitems-0.02,0.9,0.9-0.06-0.02);
  leg->SetTextSize(0.04);
  leg->AddEntry(histo_data,"Data", "EPL");
  leg->AddEntry("model","Total Fit", "L");
  leg->AddEntry("signal","J/#psi K^{#pm} Signal", "F");
  leg->AddEntry("combinatorial","Combinatorial Background", "L");
  if (ws->var("f_jpsipi")->getVal()>0.) leg->AddEntry("jpsipi","J/#psi #pi^{#pm} Background", "F");
  if (ws->var("f_nonprompt")->getVal()>0.) leg->AddEntry("nonprompt","B #rightarrow J/#psi+K+X Background", "L");
  leg->Draw();

  TLegend *leg2 = new TLegend(0.2,0.9-0.06,0.9,0.9);
  leg2->SetTextSize(0.042);
  leg2->SetTextAlign(32);
  leg2->SetHeader("full set");
  leg2->Draw();

  TLine* SRlow = new TLine(std::get<0>(SRandFrac), 0, std::get<0>(SRandFrac), 50000);
  TLine* SRhigh = new TLine(std::get<1>(SRandFrac), 0, std::get<1>(SRandFrac), 50000);

  SRlow->Draw("same");
  SRhigh->Draw("same");

  TLatex* tex = new TLatex();
  tex->SetTextSize(0.03);
  // tex->DrawLatex(config::Bplus.massMin,frame_m->GetMaximum()*1.02,"CMS Preliminary");
  tex->DrawLatex(5.5, 25000, TString::Format("f_{BG} = %g", std::get<2>(SRandFrac)));

  TLatex* tex2 = new TLatex();
  tex2->SetTextSize(0.03);
  tex2->DrawLatex(5.5, 20000, TString::Format("f_{J/#psi #pi^{#pm}} = %g", std::get<3>(SRandFrac)));

  // tex->SetTextAlign(31);
  // tex->DrawLatex(comfig::Bplus.massMax,frame_m->GetMaximum()*1.02,"49.4 pb^{-1} (13 TeV)");

  return c1;
}

// NOTE: all just preliminary here (plus assuming 3 gaussians as signal)
std::tuple<double, double, double, double> calcSRandBGFraction(RooWorkspace* ws, const double nSig = 3)
{
  using namespace RooFit;

  const double frac3 = getVarVal(ws, "m_fraction3");
  const double frac2 = getVarVal(ws, "m_fraction2");
  const double frac1 = 1 - frac2 - frac3;
  // const double frac1 = 1 - frac2;

  const double sig3 = getVarVal(ws, "m_sigma3");
  const double sig2 = getVarVal(ws, "m_sigma2");
  const double sig1 = getVarVal(ws, "m_sigma1");

  const double sigma = sig1 * frac1 + sig2 * frac2 + sig3 * frac3;
  // const double sigma = sig1 * frac1 + sig2 * frac2;

  std::cout << "====================" << std::endl;
  std::cout << "   s    |    f      " << std::endl;
  std::cout << sig1 << " | " << frac1 << std::endl;
  std::cout << sig2 << " | " << frac2 << std::endl;
  std::cout << sig3 << " | " << frac3 << std::endl;
  std::cout << "sigma = " << sigma << std::endl;

  const double mean = getVarVal(ws, "m_mean");
  const double lowM = mean - nSig * sigma;
  const double highM = mean + nSig * sigma;

  std::cout << "Signal region: " << lowM << " < M_B < " << highM << std::endl;

  auto* Bmass = static_cast<RooRealVar*>(ws->var("Bmass"));
  Bmass->setRange("SR", lowM, highM);
  const double fracSigInSR = ws->pdf("pdf_m_signal")->createIntegral(RooArgSet(*Bmass),
                                                                     NormSet(RooArgSet(*Bmass)),
                                                                     Range("SR"))->getVal();

  const double fracCombInSR = ws->pdf("pdf_m_combinatorial")->createIntegral(RooArgSet(*Bmass),
                                                                             NormSet(RooArgSet(*Bmass)),
                                                                             Range("SR"))->getVal();

  const double fracPiInSR = ws->pdf("pdf_m_jpsipi")->createIntegral(RooArgSet(*Bmass),
                                                                    NormSet(RooArgSet(*Bmass)),
                                                                    Range("SR"))->getVal();

  std::cout << "fSig = " << fracSigInSR << ", fBG = " << fracCombInSR << ", fPi = " << fracPiInSR << std::endl;

  std::cout << ws->pdf("Bmass_model")->createIntegral(RooArgSet(*Bmass),
                                                      NormSet(RooArgSet(*Bmass)),
                                                      Range("SR"))->getVal() << std::endl;

  const double allEvents = ws->data("jpsi_fromB_data_rap0_pt0")->sumEntries();
  const double fJpsiPi = getVarVal(ws, "n_jpsipi") / allEvents;
  const double fSignal = getVarVal(ws, "n_signal") / allEvents;
  const double fComb = getVarVal(ws, "n_combinatorial") / allEvents;
  // const double fNP = getVarVal(ws, "n_nonprompt") / allEvents;

  // fJpsiPi * fracPiInSr ~ 0 -> neglected here, same for nonprompt

  const double fracSig = fracSigInSR * fSignal;
  const double fracJpsiPi = fracPiInSR * fJpsiPi;
  const double fracComb = fracCombInSR * fComb;

  const double fracTotalInSR = fracSig + fracComb + fracJpsiPi; // same as integrating the Bmass_model over the SR

  std::cout << "total fraction in SR: " << fracTotalInSR << std::endl;

  std::cout << "background fraction: " << fracComb / fracTotalInSR << std::endl;
  std::cout << "JpsiPi fraction: " << fracJpsiPi / fracTotalInSR << std::endl;

  // Calculate significance (and number of signal and background events)
  const double nSigInSR = fracSigInSR * getVarVal(ws, "n_signal")
    + fracPiInSR * getVarVal(ws, "n_jpsipi");
  const double nBkgInSR = fracCombInSR * getVarVal(ws, "n_combinatorial");

  std::cout << "Number of events in Signal Region:\n";
  std::cout << "S = " << nSigInSR << ", B = " << nBkgInSR
            << " significance = " << nSigInSR / std::sqrt(nSigInSR + nBkgInSR) << '\n';

  return std::make_tuple(lowM, highM, fracComb/fracTotalInSR, fracJpsiPi / fracTotalInSR);
}

#endif
