

#include <TRestWimpSensitivity.h>
#include <TRestWimpUtils.h>
#include <chrono>


std::vector<double> logSpacedVector(double start, double end, uint numPoints, bool includeEnd = false) {
    std::vector<double> result;
    result.reserve(includeEnd ? numPoints+1 : numPoints);

    double logStart = std::log10(start);
    double logEnd = std::log10(end);
    double step = (logEnd - logStart) / numPoints;
    for (int i = 0; i < (includeEnd ? numPoints+1 : numPoints); ++i) {
        double value = std::pow(10, logStart + i * step);
        result.push_back(value);
    }

    return result;
}

std::vector<double> linearSpacedVector(double start, double end, uint numPoints, bool includeEnd = false) {
    std::vector<double> result;
    result.reserve(includeEnd ? numPoints+1 : numPoints);

    double step = (end - start) / numPoints;
    for (int i = 0; i < (includeEnd ? numPoints+1 : numPoints); ++i) {
        double value = start + i * step;
        result.push_back(value);
    }

    return result;
}

void WIMP_Sensitivity(const std::string& rmlFile, const double wimpStart = 0.01,
                           const double wimpEnd = 50, const int numPoints = 250,
                           const bool useLogScale = true) {

    std::vector<std::string> fileSelection = TRestTools::GetFilesMatchingPattern(rmlFile);
    for (auto& file : fileSelection) {
        std::cout << "Sensitivity for " << file << std::endl;
    
        TRestWimpSensitivity WS(file.c_str());
        WS.PrintMetadata();

        std::string outputFile = WS.BuildOutputFileName(".dat");

        std::ofstream ofs(outputFile, std::ofstream::out);
        if (!ofs.is_open()) {
            std::cout << "It was not possible to create file: " << outputFile << std::endl;
            return;
        }

        for (auto n : WS.GetNuclei()) {
            ofs << "# Nuclei name: " << n.fNucleusName.Data() << "\n";
            ofs << "# Atomic number: " << n.fAnum << "\n";
            ofs << "# Number of protons: " << n.fZnum << "\n";
            ofs << "# Abundance: " << n.fAbundance << "\n";
            ofs << "#\n";
        }
        ofs << "# WimpDensity: " << WS.GetWimpDensity() << " GeV/cm3" << "\n";
        ofs << "# VLab: " << WS.GetLabVelocity() << " km/s\n# VRMS: " << WS.GetRmsVelocity()
                    << " km/s\n# VEscape: " << WS.GetEscapeVelocity() << " km/s" << "\n";
        ofs << "# Exposure: " << WS.GetExposure() << " kg*day" << "\n";
        ofs << "# Background Level: " << WS.GetBackground() << " c/keV/day" << "\n";
        ofs << "# Recoil energy range: (" << WS.GetEnergySpectra().X() << ", " << WS.GetEnergySpectra().Y()
                    << ") keV\n# Step: " << WS.GetEnergySpectraStep() << " keV" << "\n";
        ofs << "# Sensitivity energy range: (" << WS.GetEnergyRange().X() << ", " << WS.GetEnergyRange().Y() << ") keV"
                    << "\n";
        ofs << "# Use quenching factor: " << (WS.GetUseQuenchingFactor() ? "true" : "false") << "\n";

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        const double vMax = WS.GetEscapeVelocity() + WS.GetLabVelocity();

        std::vector<double> wimpMasses;
        if (useLogScale)
            wimpMasses = logSpacedVector(wimpStart, wimpEnd, numPoints, true);
        else
            wimpMasses= linearSpacedVector(wimpStart, wimpEnd, numPoints, true);
        for (double wimpMass : wimpMasses) {
            const double sens = WS.GetSensitivity(wimpMass);

            if (sens > 0) {
                std::cout << "WIMP mass " << wimpMass << " " << sens << std::endl;
                ofs << wimpMass << "\t" << sens << "\n";
            } else {
                double min_E = WS.GetEnergyRange().X();
                int min_Anum = 999;
                for (auto& nucl : WS.GetNuclei()) {
                    if (nucl.fAnum < min_Anum) {
                        min_Anum = nucl.fAnum;
                    }
                }
                double min_vMin = TRestWimpUtils::GetVMin(wimpMass, min_Anum, min_E);
                std::cout << "WIMP mass " << wimpMass;
                if (min_vMin > vMax)
                    std::cout << " Warning: min_vMin > vMax.";
                else  std::cout << " Warning: not enough rate.";
                std::cout << std::endl;
            }
        }
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
    }

}
