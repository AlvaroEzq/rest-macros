//////////////////////////////////////////////////////////
/// Macro to refit the gain map with a simple 'GUI' to allow 
/// the user to add or delete peaks and update the fits with
/// the FIT PANEL. This macro is meant to be used with the
/// FIT PANEL. The GUI consists of a dialog canvas and two 
/// graphical canvases, one with all the segments and another
/// one with the segment selected alone. The dialog canvas
/// contains a button for each module. This will draw the
/// segments of the module in the first canvas (cAll). Then
/// the user can select the segment to be shown in the second
/// canvas by clicking on the first one. When the spectrum of 
/// the segment is shown in the second canvas (cAlone), the
/// user can add or delete peaks with the buttons 'Add peak'
/// and 'Delete peak'. The user can also update the fits 
/// using the FIT PANEL by selecting the fit type 'Prev. Fit'
/// and fit function named 'g'+peakNumber (in energy descending
/// order starting at 0). It is important to check the option
/// 'Add to list' in the FIT PANEL (the draw option 'SAME' is
/// optional but it can be helpful), if not all the fits will
/// be deleted. After fitting with the fit panel, click on the
/// button 'UpdateFits' to update the fits in the canvas and the
/// calibration curve of that segment.
/// Finally, export the gain map with the button 'Export' on the
/// dialog canvas. The user can also export the gain map to a
/// root file with the command 'gm.Export();' in the terminal.
///
/// \author: Álvaro Ezquerro aezquerro@unizar.es
///
/// Possible improvements:
///
//////////////////////////////////////////////////////////


TRestDataSetGainMap gm;
TRestDataSetGainMap::Module *m = nullptr;
std::map<std::string,double> meansAux={};//to store the previous fit means (for add peak functionality)

const int screenWidth = gClient->GetDisplayWidth();
const int screenHeight = gClient->GetDisplayHeight();

const int cAllWidth = 0.5*screenWidth;
const int cAllHeight = 0.7*screenHeight;
const int cAloneWidth = 0.3*screenWidth;
const int cAloneHeight = 0.45*screenHeight;

TCanvas *cAll = new TCanvas("cAll","cAll", cAllWidth, cAllHeight);
TCanvas *cAlone = new TCanvas("cAlone","cAlone", cAloneWidth, cAloneHeight);

// Functions declarations
void drawAll();
void drawAlone(const int x, const int y);
void drawWithinAll(int x, int y);
void clearCanvas(TCanvas *c, size_t n_subPad);
void highlightDrawnAlonePad(const int x, const int y);
void DeletePeak(const int x, const int y, const int peakNumber);
void AddPeak(const int x, const int y, const int peakNumber);
void UpdateFits(const int x, const int y);
void changeModule(int plane, int module);
TDialogCanvas* createDialog();

// Functions definitions
void clearCanvas(TCanvas *c, size_t n_subPad = 0) {
    c->cd(n_subPad);
    if (n_subPad == 0) {
        c->GetListOfPrimitives()->Clear();
        c->Modified();
        c->Range(0,0,1,1); // to trick the canvas to ignore "same" drawing option. 
                        // See https://root.cern/doc/master/TH1_8cxx_source.html#l03068
    } else {
        TPad *subpad = (TPad*) c->GetPad(n_subPad); // Get the pointer to the subpad
        subpad->SetFillColor(0); // if not, after updateFits, the pad (inside frame) will be highlighted
        subpad->GetListOfPrimitives()->Clear();
        subpad->Modified();
        subpad->Range(0,0,1,1); // to trick the canvas to ignore "same" drawing option. 
    }
}

void drawWithinAll(int x, int y) {
    size_t nSubPad = x+1 + m->GetNumberOfSegmentsX()*(m->GetNumberOfSegmentsY()-y-1);
    // Reset the canvas
    if (cAll->GetCanvasImp()){
        clearCanvas(cAll, nSubPad);
    } else {// if it has been closed
        cAll = new TCanvas("cAll","cAll", cAllWidth, cAllHeight);
        cAll->Divide(m->GetNumberOfSegmentsX(), m->GetNumberOfSegmentsY());
    }
    cAll->cd(nSubPad);
    m->DrawSpectrum((size_t)x, (size_t) y, true, -1, cAll);

    std::string action = (string)"drawAlone(" + std::to_string(x) + (string)"," 
                         + std::to_string(m->fSegSpectra[x].size() - 1 - y) + (string)");";
    TButton *but = new TButton("Draw alone", action.c_str(), .5,.8,.8,.88);
    but->Draw();
    cAll->Update(); // avoid having to hit 'enter' in the terminal to update it
}

void drawAlone(const int x, const int y) {

    for (TObject *obj : *m->fSegSpectra.at(x).at(y)->GetListOfFunctions()) {
        if (obj && obj->InheritsFrom(TF1::Class())) {
        	TF1 *g = (TF1*) obj;
            meansAux[(std::string)g->GetName()] = g->GetParameter(1);
        }
    }

    // Reset the canvas
    if (cAlone->GetCanvasImp()){
        clearCanvas(cAlone);
    } else // if it has been closed
        cAlone = new TCanvas("cAlone","cAlone", cAloneWidth, cAloneHeight);
    
    cAlone->cd();
    std::string canvasTitle = "hSpc_" + std::to_string(m->GetPlaneId()) 
                            + "_" + std::to_string(m->GetModuleId()) + "_" 
                            + std::to_string(x) + "_" + std::to_string(y);
    cAlone->SetTitle(canvasTitle.c_str());
    m->DrawSpectrum(x, y, true, -1, cAlone);

    std::string action = "UpdateFits(";//
    action += std::to_string(x)+(string)","+std::to_string(y) + (string)")";
    TButton *butAlone = new TButton("UpdateFits", action.c_str(), .7,.75,.9,.825);
    //butAlone->SetMethod(action.c_str());
    butAlone->Draw();

    // Add one button for each peak
    for (size_t n = 0; n<m->fEnergyPeaks.size() ; n++){
        bool peakHasFit = false;
        std::string objName = "g" + std::to_string(n);

        TF1 *f = m->fSegSpectra[x][y]->GetFunction(objName.c_str());
        if (f) {
            std::string action = "DeletePeak(";
            action += std::to_string(x)+(string)","+(string)std::to_string(y)+(string)","+(string)std::to_string(n) + (string)")";
            std::string name = "Delete energy " + DoubleToString(m->fEnergyPeaks.at(n), "%g");
            TButton *but = new TButton(name.c_str(), action.c_str(), .7,.65 - n*.05,.95,.7 - n*.05);
            but->Draw();
            peakHasFit = true;
        }

        if (!peakHasFit){
            std::string action = "AddPeak(";
            action += std::to_string(x)+(string)","+(string)std::to_string(y)+(string)","+(string)std::to_string(n) + (string)")";
            std::string name = "Add energy " + std::to_string(m->fEnergyPeaks.at(n));
            TButton *but = new TButton(name.c_str(), action.c_str(), .7,.65 - n*.05,.95,.7 - n*.05);
            but->Draw();
        }
    }
    cAlone->Update();

    // Highlight the segment in the cAll canvas
    highlightDrawnAlonePad(x, y);
}

void highlightDrawnAlonePad(const int x, const int y){
    uint COLOR = 38; // set color here (38 es azul apagado)

    // Get the number of pads inside the canvas
    size_t nPads = 0;
    for (const auto& object : *cAll->GetListOfPrimitives())
        if (object->InheritsFrom(TVirtualPad::Class())) ++nPads;

    size_t n_subPad = x+1 + m->GetNumberOfSegmentsX()*(m->GetNumberOfSegmentsY()-y-1);

    // Reset previous highlights
    for (size_t i = 0; i < nPads; i++) {
        if (i == n_subPad) continue; // skip the selected pad
        TVirtualPad *pad = (TVirtualPad*) cAll->cd(i+1);
        if (pad->GetFillColor() != 0) { // 0 is white
            pad->SetFillColor(0);
            pad->Modified();
            pad->Update();
        }
    }

    // Highlight the selected pad
    if (n_subPad > nPads) {
        std::cout << "Error: the number of pads is " << nPads << " and the selected pad is " << n_subPad << std::endl;
        return;
    }
    TVirtualPad *pad = (TVirtualPad*) cAll->cd(n_subPad);
    if (pad->GetFillColor() == COLOR) return; // no need to modify it to same color
    pad->SetFillColor(COLOR);
    pad->Modified();
    pad->Update();
    /*
    cAll->cd(n_subPad);
    gPad->SetFillColor(kYellow);
    gPad->Modified();
    gPad->Update();
    */
}

void DeletePeak(const int x, const int y, const int peakNumber){
    TH1F* h = m->fSegSpectra.at(x).at(y);
    TGraph* gr = m->fSegLinearFit.at(x).at(y);
    std::string objName = "g" + std::to_string(peakNumber);
    TF1* f = h->GetFunction(objName.c_str());
    if (f){
        //std::cout << "Deleting peak " << peakNumber << " in segment " << x << "," << y << std::endl;
        h->GetListOfFunctions()->Remove(f);
        gr->RemovePoint(peakNumber); // unnecessary as UpdateCalibrationFits will remove it
    }
    
    drawWithinAll(x,y); // Update the canvas with all the segments
    drawAlone(x,y); // Update the canvas alone
}

void AddPeak(const int x, const int y, const int peakNumber){
    TH1F* h = m->fSegSpectra.at(x).at(y);
    std::string objName = "g" + std::to_string(peakNumber);
    TF1* g = new TF1(objName.c_str(), "gaus", meansAux[objName]*0.8, meansAux[objName]*1.2);
    h->Fit(g, "R+Q0");
    //segSpectraCopy.at(peakNumber).Copy(*g);
    //h->GetListOfFunctions()->Add(g); //already added by fitting with options '+'
    //UpdateCalibrationFit(x, y, peakNumber, g->GetParameter(1));
    
    drawWithinAll(x,y); // Update the canvas with all the segments
    drawAlone(x,y); // Update the canvas alone
}

void UpdateFits(const int x, const int y){
    TH1F* h = m->fSegSpectra.at(x).at(y);

    TList* list = h->GetListOfFunctions();
    for (size_t n = 0; n<m->fEnergyPeaks.size() ; n++){
        // find the last TF1 named gn because FitPanel with "AddToList" option
        // will add another fit with the same name at last position
        std::string objName = "g" + std::to_string(n);
        TF1* firstFit = h->GetFunction(objName.c_str());
        TF1* lastFit = nullptr;
        for (int i = list->GetSize() - 1; i >= 0; i--) {
            TObject* obj = list->At(i);
            if (obj && obj->InheritsFrom(TF1::Class()) && obj->GetName() == objName) {
                if (!lastFit)
                    lastFit = (TF1*) list->Remove(obj);
                else
                    list->Remove(obj);
            }
        }
        
        if (firstFit && lastFit && firstFit != lastFit)
            lastFit->Copy(*firstFit);
        if (firstFit)
            h->GetListOfFunctions()->Add(firstFit);

        if (h->GetFunction(objName.c_str()))
            //m->UpdateCalibrationFit(x, y, n, h->GetFunction(objName.c_str())->GetParameter(1)); //this function is not in the framework repository
            m->UpdateCalibrationFits(x, y);
    }
    //cAlone->Modified();
    drawWithinAll(x,y); // Update the canvas with all the segments
    drawAlone(x,y); // Update the canvas alone. Done after drawWithinAll to highlight the pad at cAll
    
    // --- PRINT INFO ---
    std::cout << std::endl;
    std::cout << "Segment " << x << ", " << y << std::endl;
    // print all the functions of the histogram
    for (TObject *obj : *h->GetListOfFunctions()) {
        if (obj && obj->InheritsFrom(TF1::Class())) {
            TF1 *f = (TF1 *) obj;
            std::cout << "Function " << f->GetName() << std::endl;
            std::cout << "\tmean : " << f->GetParameter(1) << std::endl;
            std::cout << "\tsigma : " << f->GetParameter(2) << std::endl;
        }
    }
    
    // print all the points of the graph
    TGraph* gr = m->fSegLinearFit.at(x).at(y);
    if (!gr) return;
    std::cout << "Graph " << gr->GetName() << std::endl;
    for (int i = 0; i < gr->GetN(); i++) {
        double x, y;
        gr->GetPoint(i, x, y);
        std::cout << "\tPoint " << i << ": " << x << ", " << y << std::endl;
    }
    // print all the functions of the histogram
    for (TObject *obj : *gr->GetListOfFunctions()) {
        if (obj && obj->InheritsFrom(TF1::Class())) {
            TF1 *f = (TF1 *) obj;
            std::cout << "Function " << f->GetName() << std::endl;
            std::cout << "\tslope : " << f->GetParameter(1) << std::endl;
            std::cout << "\tintercept : " << f->GetParameter(0) << std::endl;
        }
    }
    std::cout << std::endl;
}

void drawAll() {
    cAll->cd();
    // Reset the canvas
    if (cAll->GetCanvasImp()){
        clearCanvas(cAll);
    } else // if it has been closed
        cAll = new TCanvas("cAll","cAll",900,700);

    m->DrawSpectrum(true,-1, cAll);
    // Add all the 'draw alone' buttons
    for (size_t i = 0; i <m->fSegSpectra.size(); i++) {
        for (size_t j = 0; j <m->fSegSpectra[i].size(); j++) {
            cAll->cd(i + 1 +m->fSegSpectra[i].size() * j);
            /*std::string action = (string)"m->DrawSpectrum((size_t)" + std::to_string(i) + (string)",(size_t)" + std::to_string(m->fSegSpectra[i].size() - 1 - j)
             + (string)", true, -1, cAlone);";//*/
            std::string action = (string)"drawAlone(" + std::to_string(i) + (string)"," + std::to_string(m->fSegSpectra[i].size() - 1 - j) + (string)");";
            TButton *but = new TButton("Draw alone", action.c_str(), .5,.8,.8,.88);
            but->Draw();
        }
    }
    cAll->Update();
}

void changeModule(int plane, int module){
    // Updating the fits of all segments of the previous module
    if (m) {
        for (size_t i = 0; i <m->fSegSpectra.size(); i++) 
            for (size_t j = 0; j <m->fSegSpectra[i].size(); j++) 
                m->UpdateCalibrationFits(i, j);
    }
    m = gm.GetModule(plane,module);
    if(!m) return;

    // Reset the canvas cAlone
    if (cAlone->GetCanvasImp()){
        clearCanvas(cAlone);
        cAlone->SetTitle("cAlone");
        cAlone->Update();
    }

    drawAll();
    cAll->SetTitle(((std::string)"Plane " + std::to_string(plane) + (std::string)", Module " + std::to_string(module)).c_str());
    cAll->Update();
}

TDialogCanvas* createDialog(){
    double width = 300, height = 100;
    int nPlanes = gm.GetNumberOfPlanes();
    int nModules = gm.GetModuleIDs(*gm.GetPlaneIDs().begin()).size();
    width = width * nPlanes;
    height = height * nModules;
    TDialogCanvas *dialog = new TDialogCanvas("Module selection","",width,height);
    dialog->GetCanvasImp()->SetWindowPosition(25,50);
    // Add buttons to dialog
    int i=0;
    for (auto pm : gm.GetPlaneIDs()) {
        int j=0;
        for (auto mm : gm.GetModuleIDs(pm)) {
            std::string action = "changeModule(";//
            action += std::to_string(pm)+(string)","+(string)std::to_string(mm) + (string)");";
            std::string name = "Plane " + std::to_string(pm) + ", Module " + std::to_string(mm);
            
            // create button at position i,j in table of buttons inside dialog
            TButton *but = new TButton(name.c_str(), action.c_str(), .1 + i*.8/nPlanes,.4 + j*.4/nModules,.1 + (i+1)*.8/nPlanes,.2 + (j+1)*.4/nModules);
            but->Draw();
            j++;
        }
        i++;
    }

    TButton *butExport = new TButton("Export", "gm.Export();", .4,.1,.6,.3);
    butExport->Draw();
    return dialog;
}

void refitGainMap(std::string gainMapFile = "data/testing_Cal_D01983_Hits_ThresholdIntegral_4x4.root"){
    //TFile* f = new TFile("data/testing_Cal_D01983_Hits_ThresholdIntegral_4x4.root");
    //gm = *((TRestDataSetGainMap*)  f->Get("calRaw4x4")); 
    gm.Import(gainMapFile); // It doesnt work with gm->Import(). Why?

    std::cout << "************************************************" << std::endl;
    std::cout << "*********** REFITTING MACRO ********************" << std::endl;
    // Resize the above messages to fit this new scheme
    std::cout << "* This macro is meant to be used with the FIT  *" << std::endl;
    std::cout << "* PANEL. To proper use it, open the FIT PANEL  *" << std::endl;
    std::cout << "* in the Tools menu of the small canvas. Once  *" << std::endl;
    std::cout << "* in the fit panel, select the fit type        *" << std::endl;
    std::cout << "* 'Prev. Fit and fit function named            *" << std::endl;
    std::cout << "*'g'+peak number (in energy descending order   *" << std::endl;
    std::cout << "* starting at 0). For example                  *" << std::endl;
    std::cout << "* g0 is 22.5keV and g1 is 8.0keV. It is        *" << std::endl;
    std::cout << "* important to check the option 'Add to list'  *" << std::endl;
    std::cout << "* in the FIT PANEL (the draw option 'SAME' is  *" << std::endl;
    std::cout << "* optional but it can be helpful)              *" << std::endl;
    std::cout << "* After fitting with the fit panel, click on   *" << std::endl;
    std::cout << "* the button 'UpdateFits' to update the fits   *" << std::endl;
    std::cout << "* in the canvas and the calibration curve of   *" << std::endl;
    std::cout << "* that segment.                                *" << std::endl;
    std::cout << "************************************************" << std::endl;

    auto dialog = createDialog();

    // Set the position of the canvases
    Int_t x = 0, y = 0;
    UInt_t w = 0, h = 0;
    dialog->GetCanvasImp()->GetWindowGeometry(x,y,w,h);

    cAll->SetWindowPosition( x, y + 35 + 100*gm.GetModuleIDs(*gm.GetPlaneIDs().begin()).size());
    Int_t x2 = 0, y2 = 0;
    UInt_t w2 = 0, h2 = 0;
    cAll->GetCanvasImp()->GetWindowGeometry(x2,y2,w2,h2);

    cAlone->SetWindowPosition( x2 + w2, y2);
    
}
