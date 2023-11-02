

#include <TRestWimpSensitivity.h>

void REST_WIMP_RecoilRate(const std::string& rmlFile, const double wimpMass = 1,
                          const double crossSection = 1E-45) {
    TRestWimpSensitivity WS(rmlFile.c_str());
    WS.PrintMetadata();
    for (const auto& nucleus : WS.GetNuclei())
        if (nucleus.fAbundance != 1)
            std::cout << "WARNING: " << nucleus.fNucleusName << " abundance is different than 1."
                      << " Then, the rate calculated is not in c/keV/day/kg(of " << nucleus.fNucleusName
                      << ") but in c/keV/day/(kg*abundance)(of " << nucleus.fNucleusName
                      << ")." << std::endl; //This can be right if the "per kg" refers to kg of the whole mixture, but in this case the sum of the abundances must be 1.
                      
    auto recoilRate = WS.GetRecoilSpectra(wimpMass, crossSection);

    std::stringstream ss;
    ss << "WimpSensitivity_WimpMass_" << wimpMass << "_CrossSect_" << crossSection;

    TCanvas* can = new TCanvas(ss.str().c_str(), ss.str().c_str());
    can->SetLogy();
    can->SetLogx();
    TLegend* leg = new TLegend(0.8, 0.7, 0.9, 0.9);

    int color = 1;

    //get the histogram range
    double max = 0;
    for (const auto& [element, hist] : recoilRate)
        if (hist->GetMaximum() > max)
            max = hist->GetMaximum();
    double min = max;
    for (const auto& [element, hist] : recoilRate)
        if (hist->GetMinimum() < min)
            min = hist->GetMinimum();

    for (const auto& [element, hist] : recoilRate) {
        hist->SetTitle(("Recoil spectra m="+DoubleToString(wimpMass, "%g")+" GeV").c_str());
        hist->GetXaxis()->SetTitle("Energy (keV)");
        hist->GetYaxis()->SetTitle("Recoil rate (c/keV/kg/day)");
        hist->SetAxisRange(max*1.E-9, max*5, "Y");
        //hist->SetAxisRange(1.E-14, 1.E-1, "Y");
        hist->SetLineColor(color);
        hist->SetLineWidth(3);
        hist->SetStats(false);
        leg->AddEntry(hist, element.c_str());
        can->cd();
        hist->Draw("SAME");
        color++;
    }

    can->cd();
    leg->Draw();
    //can->SetFrameLineWidth(3);
    can->RedrawAxis();

    /*
    WS.CalculateQuenchingFactor();
    auto qF = WS.GetQuenchingFactor();
    ss << "_ee";

    TCanvas* can_ee = new TCanvas(ss.str().c_str(), ss.str().c_str());
    can_ee->SetLogy();
    can_ee->SetLogx();
    TLegend* leg_ee = new TLegend(0.1, 0.7, 0.3, 0.9);

    color = 1;
    can_ee->cd();
    for (const auto& [element, hist] : recoilRate) {
        TH1D* hist_ee = (TH1D*) hist->Clone((element+"_ee").c_str());
        for (int i = 1; i <= hist_ee->GetNbinsX(); i++) 
            hist_ee->SetBinContent(i, hist_ee->GetBinContent(i) * qF[element]->GetBinContent(i));
        
        hist->SetTitle(("Recoil spectra m="+DoubleToString(wimpMass, "%g")+" GeV").c_str());
        hist->GetXaxis()->SetTitle("Energy (keVee)");
        hist->GetYaxis()->SetTitle("Recoil rate (c/keVee/kg/day)");
        //hist->SetAxisRange(1.E-6, 0.01, "Y");
        hist->SetLineColor(color);
        hist->SetLineWidth(3);
        leg->AddEntry(hist, element.c_str());
        can->cd();
        hist->Draw("SAME");
        color++;
    }
    
    can_ee->cd();
    leg_ee->Draw();
    */
}
