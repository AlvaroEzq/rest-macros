
/////////////////////////////////////////////////////////////////////////
/// This macro copies (appends) all the objects from a set of root files 
/// into a single file. 
///
/// ### Parameters
/// * **inputFileName**: pattern of the names of the files to copy the
///                      objects from.
///                      See TRestTools::GetFilesMatchingPattern().
/// * **outputFileName**: name of the file to copy the objects to.
///
/// \author: Álvaro Ezquerro aezquerro@unizar.es
/////////////////////////////////////////////////////////////////////////

void copyObjects(const std::string& inputFileName, const std::string& outputFileName)
{
    
    TFile* outputFile = new TFile(outputFileName.c_str(), "UPDATE");
    
    if (outputFile == nullptr) {
        std::cout << "Error: cannot open file " << outputFileName << std::endl;
        return;
    }

    std::vector<std::string> inputFileSelection =
            TRestTools::GetFilesMatchingPattern(inputFileName);

    for (const auto& file : inputFileSelection) {
       
        TFile* inputFile = new TFile(inputFileName.c_str(), "READ");
        if (inputFile == nullptr) {
            std::cout << "Error: cannot open file " << inputFileName << std::endl;
            continue;
        }

        std::cout << "From file " << file << std::endl;
        std::cout << "Copying objects:" << std::endl;
        TIter nextkey(inputFile->GetListOfKeys());
        TKey* key;
        while ((key = (TKey*)nextkey())) {
            TObject *obj = key->ReadObj();
            outputFile->cd();
            std::cout << "\t" << key->GetClassName() << "\t" << obj->GetName() << std::endl;
            obj->Write();
            delete obj;
        }

        inputFile->Close();
    }

    outputFile->Close(); 
}