
/**
 * @brief Creates a data set from a file pattern and saves it to an output file.
 *      The data set is created with all the observables from the TRestAnalysisTree
 *      of the first file matching the pattern.
 * 
 * @param filePattern The pattern of the input files to be used to create the data set.
 * @param outputFileName The name of the output file where the data set will be saved.
 */
void createDataSet(std::string filePattern = "./data/R01850_*_Hits_Calibration_109Cd_Both_cronTREX_V2.3.13.root", std::string outputFileName = ""){
    
    TRestDataSet ds;
    ds.SetName("ds"); //si no tiene nombre (el TObject), luego no se puede leer del archivo .root
    std::vector<std::string> fileSelection = TRestTools::GetFilesMatchingPattern(filePattern);
    if (fileSelection.empty()){
        std::cout << "ERROR: No files found with pattern " << filePattern << std::endl;
        return;
    }
    ds.SetFilePattern(filePattern);

    //Get the list of all observables from first file
    TFile f(fileSelection.at(0).c_str());
    TRestAnalysisTree *tree = (TRestAnalysisTree*) f.Get<TRestAnalysisTree>("AnalysisTree");
    if (tree==nullptr){
        std::cout << "ERROR: No AnalysisTree with name \"AnalysisTree\" found in file " << fileSelection.at(0) << std::endl;
        return;
    }
    std::vector<std::string> obsList = tree->GetObservableNames();
    if (obsList.empty()){
        std::cout << "ERROR: no observables found in AnalysisTree at file " << fileSelection.at(0) << std::endl;
        return;
    }
    ds.SetObservablesList(obsList);

    //Generate data set
    ds.GenerateDataSet();

    //Autoset outputFileName (based on typical R{runNumber}_{subrunNumber}_{tags}.root format) to D{runNumber}_{tags}.root
    if (outputFileName.empty()){
        auto [path, fName] = TRestTools::SeparatePathAndName(fileSelection.at(0));
        fName = fName.erase(7,6); //Get rid of subrunNumber e.g. R01850_00000_Hits.root -> R01850_Hits.root
        fName = "D" + fName.erase(0,1); //Change R for D e.g. R01850_Hits.root -> D01850_Hits.root
        if (!path.empty())
            path = path + "/";
        outputFileName = path + fName;
        std::cout << "Warning: no outputFileName defined. Autosetting to " << outputFileName << std::endl;
    }
    ds.Export(outputFileName);

}