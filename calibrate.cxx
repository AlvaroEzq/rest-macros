/////////////////////////////////////////////////////////////////////////
/// This macro performs multiple calibrations (following the parameters 
/// of the set rmlFiles) of a single dataSet file (dataSetToCalibrate).
///
/// For each rml file (following rmlFile pattern) it will calculate the
/// calibration parameters and export them to outputFileName (given
/// at the rml file as a paremeter of the TRestDataSetGainMap). This 
/// is done unless a file with outputFileName already exists, in which
/// case it will import the calibration parameters from that file.
///
/// If dataSetToCalibrate is empty, it will only calculate (or load)
/// the calibration parameters following the rml parameters.
///
/// ### Parameters
/// * **rmlFile**: pattern of the names of the rml configuration files for
/// the TRestDataSetGainMap class definition. (See 
/// TRestTools::GetFilesMatchingPattern() ).
/// * **dataSetToCalibrate**: name of the file to be calibrated.
///
/// \author: Álvaro Ezquerro aezquerro@unizar.es
/////////////////////////////////////////////////////////////////////////

void calibrate(const std::string& rmlFile, const std::string& dataSetToCalibrate = "") {

    if ( !dataSetToCalibrate.empty() && !TRestTools::fileExists(dataSetToCalibrate) ){
        std::cout << "File " << dataSetToCalibrate << " does not exist" << std::endl;
        return;
    }
    /*else if ( !TRestTools::isDataSet(dataSetToCalibrate)) { 
        std::cout << "File " << dataSetToCalibrate << " is not a data set" << std::endl;
        return;
    }//*/ //it may not be needed to be a TRestDataSet as it may function as weel with TRestAnalysisTree

    std::vector<std::string> fileSelection = TRestTools::GetFilesMatchingPattern(rmlFile);
    for (auto& file : fileSelection) {
        std::cout << "Sensitivity for " << file << std::endl;

        TRestDataSetGainMap cal(file.c_str());
        if ( TRestTools::fileExists( cal.GetOutputFileName() ) ) {
            std::cout << "Calibration file for " << file << " exists in " << cal.GetOutputFileName() << std::endl;
            cal.Import(cal.GetOutputFileName());
        } else {
            cal.SetVerboseLevel( (TRestStringOutput::REST_Verbose_Level) 1); //0:essential 1: warnings, 2: info, 3: debug
            cal.Calibrate();
            std::cout << "Calibration file for " << file << " saved in " << cal.GetOutputFileName() << std::endl;
            cal.Export();
        }
        if ( !dataSetToCalibrate.empty() )
            cal.CalibrateDataSet(dataSetToCalibrate);
    }
        
}