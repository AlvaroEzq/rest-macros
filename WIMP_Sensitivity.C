

#include <TRestWimpSensitivity.h>
#include <TRestWimpUtils.h>
#include <chrono>

void WIMP_Sensitivity(const std::string& rmlFile, const double wimpStart = 0.01,
                           const double wimpEnd = 50, const double wimpStep = 0.1) {

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

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        const double vMax = WS.GetEscapeVelocity() + WS.GetLabVelocity();
        double step = wimpStep;
        for (double wimpMass = wimpStart; wimpMass <= wimpEnd; wimpMass += step) {
            if (wimpMass < 0.1) step = wimpStep /100;
            if (0.1 <= wimpMass &&wimpMass < 1) step = wimpStep /10;
            if (1 <= wimpMass && wimpMass < 10) step = wimpStep;
            if ( 9.999 <= wimpMass ) step = wimpStep*10;

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
