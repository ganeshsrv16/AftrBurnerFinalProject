#include "GLViewGaneshFinalProject.h"

#include "WorldList.h" //This is where we place all of our WOs
#include "ManagerOpenGLState.h" //We can change OpenGL State attributes with this
#include "Axes.h" //We can set Axes to on/off with this
#include "PhysicsEngineODE.h"

//Different WO used by this module
#include "WO.h"
#include "WOStatic.h"
#include "WOStaticPlane.h"
#include "WOStaticTrimesh.h"
#include "WOTrimesh.h"
#include "WOHumanCyborg.h"
#include "WOHumanCal3DPaladin.h"
#include "WOWayPointSpherical.h"
#include "WOLight.h"
#include "WOSkyBox.h"
#include "WOCar1970sBeater.h"
#include "Camera.h"
#include "CameraStandard.h"
#include "CameraChaseActorSmooth.h"
#include "CameraChaseActorAbsNormal.h"
#include "CameraChaseActorRelNormal.h"
#include "Model.h"
#include "ModelDataShared.h"
#include "ModelMesh.h"
#include "ModelMeshDataShared.h"
#include "ModelMeshSkin.h"
#include "WONVStaticPlane.h"
#include "WONVPhysX.h"
#include "WONVDynSphere.h"
#include "WOImGui.h" //GUI Demos also need to #include "AftrImGuiIncludes.h"
#include "AftrImGuiIncludes.h"
#include "AftrGLRendererBase.h"
#include "curl/curl.h"
#include <iostream>
#include <json/json.h>
#include "json.hpp"
#include <fstream>
#include "WOGridECEFElevation.h"
#include "AftrUtilities.h"
#include <algorithm>
#include "GLSLShader.h"
#include "GLSLShaderPerVertexColorGL32.h"


using namespace Aftr;
using json = nlohmann::json;
size_t curl_write_callback_string(void* contents, size_t size, size_t nmemb, std::string* s);
GLViewGaneshFinalProject* GLViewGaneshFinalProject::New( const std::vector< std::string >& args )
{
   GLViewGaneshFinalProject* glv = new GLViewGaneshFinalProject( args );
   glv->init( Aftr::GRAVITY, Vector( 0, 0, -1.0f ), "aftr.conf", PHYSICS_ENGINE_TYPE::petODE );
   glv->onCreate();
   return glv;
}


GLViewGaneshFinalProject::GLViewGaneshFinalProject( const std::vector< std::string >& args ) : GLView( args )
{
   //Initialize any member variables that need to be used inside of LoadMap() here.
   //Note: At this point, the Managers are not yet initialized. The Engine initialization
   //occurs immediately after this method returns (see GLViewGaneshFinalProject::New() for
   //reference). Then the engine invoke's GLView::loadMap() for this module.
   //After loadMap() returns, GLView::onCreate is finally invoked.

   //The order of execution of a module startup:
   //GLView::New() is invoked:
   //    calls GLView::init()
   //       calls GLView::loadMap() (as well as initializing the engine's Managers)
   //    calls GLView::onCreate()

   //GLViewGaneshFinalProject::onCreate() is invoked after this module's LoadMap() is completed.
}


void GLViewGaneshFinalProject::onCreate()
{
   //GLViewGaneshFinalProject::onCreate() is invoked after this module's LoadMap() is completed.
   //At this point, all the managers are initialized. That is, the engine is fully initialized.

   if( this->pe != NULL )
   {
      //optionally, change gravity direction and magnitude here
      //The user could load these values from the module's aftr.conf
      this->pe->setGravityNormalizedVector( Vector( 0,0,-1.0f ) );
      this->pe->setGravityScalar( Aftr::GRAVITY );
   }
   this->setActorChaseType( STANDARDEZNAV ); //Default is STANDARDEZNAV mode
   //this->setNumPhysicsStepsPerRender( 0 ); //pause physics engine on start up; will remain paused till set to 1
}


GLViewGaneshFinalProject::~GLViewGaneshFinalProject()
{
   //Implicitly calls GLView::~GLView()
}


void GLViewGaneshFinalProject::updateWorld()
{
   GLView::updateWorld(); //Just call the parent's update world first.
                          //If you want to add additional functionality, do it after
                          //this call.


}


void GLViewGaneshFinalProject::onResizeWindow( GLsizei width, GLsizei height )
{
   GLView::onResizeWindow( width, height ); //call parent's resize method.
}


void GLViewGaneshFinalProject::onMouseDown( const SDL_MouseButtonEvent& e )
{
   GLView::onMouseDown( e );
}


void GLViewGaneshFinalProject::onMouseUp( const SDL_MouseButtonEvent& e )
{
   GLView::onMouseUp( e );
}


void GLViewGaneshFinalProject::onMouseMove( const SDL_MouseMotionEvent& e )
{
   GLView::onMouseMove( e );
}


void GLViewGaneshFinalProject::onKeyDown( const SDL_KeyboardEvent& key )
{
   GLView::onKeyDown( key );
   if( key.keysym.sym == SDLK_0 )
      this->setNumPhysicsStepsPerRender( 1 );

   if( key.keysym.sym == SDLK_1 )
   {

   }
}


void GLViewGaneshFinalProject::onKeyUp( const SDL_KeyboardEvent& key )
{
   GLView::onKeyUp( key );
}


void Aftr::GLViewGaneshFinalProject::loadMap()
{
   this->worldLst = new WorldList(); //WorldList is a 'smart' vector that is used to store WO*'s
   this->actorLst = new WorldList();
   this->netLst = new WorldList();

   ManagerOpenGLState::GL_CLIPPING_PLANE = 1000.0;
   ManagerOpenGLState::GL_NEAR_PLANE = 0.1f;
   ManagerOpenGLState::enableFrustumCulling = false;
   Axes::isVisible = true;
   this->glRenderer->isUsingShadowMapping( false ); //set to TRUE to enable shadow mapping, must be using GL 3.2+
   this->cam->setPosition( 50,50,250 );
   this->cam->setCameraLookDirection(Vector(50, 50, 0));
   std::string shinyRedPlasticCube( ManagerEnvironmentConfiguration::getSMM() + "/models/cube4x4x4redShinyPlastic_pp.wrl" );
   std::string wheeledCar( ManagerEnvironmentConfiguration::getSMM() + "/models/rcx_treads.wrl" );
   std::string grass( ManagerEnvironmentConfiguration::getSMM() + "/models/grassFloor400x400_pp.wrl" );
   std::string human( ManagerEnvironmentConfiguration::getSMM() + "/models/human_chest.wrl" );
   
   //SkyBox Textures readily available
   std::vector< std::string > skyBoxImageNames; //vector to store texture paths
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_water+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_dust+6.jpg" );
   skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_mountains+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_winter+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/early_morning+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_afternoon+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_cloudy+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_cloudy3+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_day+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_day2+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_deepsun+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_evening+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_morning+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_morning2+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_noon+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_warp+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_Hubble_Nebula+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_gray_matter+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_easter+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_hot_nebula+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_ice_field+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_lemon_lime+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_milk_chocolate+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_solar_bloom+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_thick_rb+6.jpg" );

   {
      //Create a light
      float ga = 0.1f; //Global Ambient Light level for this module
      ManagerLight::setGlobalAmbientLight( aftrColor4f( ga, ga, ga, 1.0f ) );
      WOLight* light = WOLight::New();
      light->isDirectionalLight( true );
      light->setPosition( Vector( 0, 0, 100 ) );
      //Set the light's display matrix such that it casts light in a direction parallel to the -z axis (ie, downwards as though it was "high noon")
      //for shadow mapping to work, this->glRenderer->isUsingShadowMapping( true ), must be invoked.
      light->getModel()->setDisplayMatrix( Mat4::rotateIdentityMat( { 0, 1, 0 }, 90.0f * Aftr::DEGtoRAD ) );
      light->setLabel( "Light" );
      worldLst->push_back( light );
   }

   {
      //Create the SkyBox
      WO* wo = WOSkyBox::New( skyBoxImageNames.at( 0 ), this->getCameraPtrPtr() );
      wo->setPosition( Vector( 0, 0, 0 ) );
      wo->setLabel( "Sky Box" );
      wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
      worldLst->push_back( wo );
   }

   { 
      ////Create the infinite grass plane (the floor)
      WO* wo = WO::New( grass, Vector( 1, 1, 1 ), MESH_SHADING_TYPE::mstFLAT );
      wo->setPosition( Vector( 0, 0, 0 ) );
      wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
      wo->upon_async_model_loaded( [wo]()
         {
            ModelMeshSkin& grassSkin = wo->getModel()->getModelDataShared()->getModelMeshes().at( 0 )->getSkins().at( 0 );
            grassSkin.getMultiTextureSet().at( 0 ).setTexRepeats( 5.0f );
            grassSkin.setAmbient( aftrColor4f( 0.4f, 0.4f, 0.4f, 1.0f ) ); //Color of object when it is not in any light
            grassSkin.setDiffuse( aftrColor4f( 1.0f, 1.0f, 1.0f, 1.0f ) ); //Diffuse color components (ie, matte shading color of this object)
            grassSkin.setSpecular( aftrColor4f( 0.4f, 0.4f, 0.4f, 1.0f ) ); //Specular color component (ie, how "shiney" it is)
            grassSkin.setSpecularCoefficient( 10 ); // How "sharp" are the specular highlights (bigger is sharper, 1000 is very sharp, 10 is very dull)
         } );
      wo->setLabel( "Grass" );
      worldLst->push_back( wo );
   }

   //Make a Dear Im Gui instance via the WOImGui in the engine... This calls
   //the default Dear ImGui demo that shows all the features... To create your own,
   //inherit from WOImGui and override WOImGui::drawImGui_for_this_frame(...) (among any others you need).
   WOImGui* gui = WOImGui::New(nullptr);
   gui->setLabel("My Gui");
   gui->subscribe_drawImGuiWidget(
       [this, gui]() //this is a lambda, the capture clause is in [], the input argument list is in (), and the body is in {}
       {
   ImGui::Begin("NOAA Weather GUI");
   const char* items[] = { "---Select a region---","Ohio", "Michigan", "California", "Colorado","Texas"};
   static int item_current = 0;
   ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 1.0f, 1.0f));
   if (ImGui::Combo("##combo", &item_current, items, IM_ARRAYSIZE(items))) {
       // Handle selection of each item in the dropdown
       switch (item_current) {
       case 1:
           lat = 39.983334;
           lon = -82.983330;
           break;
       case 2: 
           lat = 45.41816514172032;
           lon = -84.62208688242518;
           break;
       case 3:
           lat = 36.746841;
           lon = -119.772591;
           break;
       case 4:
           lat = 39.113014;
           lon = -105.358887;
           break;
       case 5:
           lat = 32.779167;
           lon = -96.808891;
           break;
       default:
           lat = 0;
           lon = 0;
           // Handle error case
           break;
       }
   }
   ImGui::PopStyleColor();
   if (ImGui::Button("Enter")) {
       getChunk(lat, lon);
   }
   ImGui::SameLine(0, 20);
   if (ImGui::Button("Reset")) {
       item_current = 0;
       lat = 0;
       lon = 0;
       deleteGrid();
   }
   ImGui::End();
       });
   // Styling ImGui
   ImGuiStyle& style = ImGui::GetStyle();
   style.Colors[ImGuiCol_WindowBg] = ImVec4(0.2f, 0.2f, 0.2f, 0.3f);
   style.Colors[ImGuiCol_Button] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
   this->worldLst->push_back(gui);

}

void GLViewGaneshFinalProject::getChunk(double lat, double lon) {
    // if grid already exists removing it - Overriding
   int id =  worldLst->getIndexOfWO(this->heightField);
   if (id) {
       worldLst->eraseViaWOIndex(id);
   }
    // Construct the payload as a JSON object
    json payload = {
        {"lat", lat},
        {"long", lon}
    };

    // Convert payload to string
    std::string payloadStr = payload.dump();

    // Set up endpoint URL - LAWN ApiServer (local)
    std::string endpoint = "http://127.0.0.1:5000/chunk";

    // Initialize curl session
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "curl_easy_init() failed\n";
        return;
    }

    // Set up curl options
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payloadStr.size());
    curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());

    // Set up buffer to store the response
    std::string responseBuffer;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_callback_string);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);

    // Perform the HTTP request
    CURLcode res = curl_easy_perform(curl);

    // Check for errors
    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
        return;
    }

    // Clean up
    curl_easy_cleanup(curl);

    // Parse the response JSON
    json responseJson = json::parse(responseBuffer);

    // Extract the chunks array
    json chunksJson = responseJson["chunk"];
    int rows = chunksJson.size();
    int cols = chunksJson[0].size();
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i][j] = chunksJson[i][j];
        }
    }
    this->createGrid(matrix);
}



size_t curl_write_callback_string(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
    }
    catch (std::bad_alloc& e) {
        //handle memory problem
        return 0;
    }
    return newLength;
}
void GLViewGaneshFinalProject::createGrid(std::vector<std::vector<double>> matrix) {
    std::vector<std::vector<Aftr::VectorD>> gridpt;
    std::vector<std::vector<Aftr::aftrColor4ub>> color;

    gridpt.clear();
    color.clear();

    const int numRows = matrix.size();
    const int numCols = matrix[0].size();

    for (int i = 0; i < numRows; ++i) {
        gridpt.push_back(std::vector<Aftr::VectorD>(0));
        color.push_back(std::vector<Aftr::aftrColor4ub>(0));
        gridpt[i].clear();
        color[i].clear();
        gridpt[i].resize(numCols);
        color[i].resize(numCols);


        /*  Below 273.15 K: Blue
              273.15 K to 283.15 K : Light Blue
              283.15 K to 293.15 K : Green
              293.15 K to 303.15 K : Yellow
              303.15 K to 313.15 K : Orange
              Above 313.15 K : Red

              Below 32 °F: Blue
              32 °F to 50 °F: Light Blue
              50 °F to 68 °F: Green
              68 °F to 86 °F: Yellow
              86 °F to 104 °F: Orange
              Above 104 °F: Red
              */
        for (int j = 0; j < numCols; ++j) {
            gridpt[i][j] = VectorD(i, j, matrix[i][j]);

            double temperature = gridpt[i][j].z;
            float value = 0;
            if (temperature < 273.15) { // Below 32°F
                value = 0;
            }
            else if (temperature < 283.15) { // 32°F to 50°F
                value = (temperature - 273.15) / (283.15 - 273.15);
            }
            else if (temperature < 293.15) { // 50°F to 68°F
                value = (temperature - 283.15) / (293.15 - 283.15);
            }
            else if (temperature < 303.15) { // 68°F to 86°F
                value = (temperature - 293.15) / (303.15 - 293.15);
            }
            else if (temperature < 313.15) { // 86°F to 104°F
                value = (temperature - 303.15) / (313.15 - 303.15);
            }
            else { // Above 104°F
                value = 1;
            }
            Vector hsv = { 240 * value / 360, 1, 1 }; // Use hue value to map from red (240) to dark blue (0)
            color[i][j] = AftrUtilities::convertHSVtoRGB(hsv);
        }
    }
         // Creating a WOGrid with temperature value as height points (z axis) lat and lon as x and y axis
    this->heightField = WOGrid::New(gridpt, Vector(1, 1, 1), color);
    this->heightField->setPosition(Vector(50, 50, -200));
    ModelMeshSkin& skin = heightField->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0);
    skin.getMultiTextureSet().at(0).setTexRepeats(5.0f);
    skin.setAmbient(aftrColor4f(0.4f, 0.4f, 0.4f, 1.0f)); //Color of object when it is not in any light
    skin.setDiffuse(aftrColor4f(1.0f, 1.0f, 1.0f, 1.0f)); //Diffuse color components (ie, matte shading color of this object)
    skin.setSpecular(aftrColor4f(0.4f, 0.4f, 0.4f, 1.0f)); //Specular color com	ponent (ie, how "shiney" it is)
    skin.setSpecularCoefficient(10);
    std::string vertexShader = ManagerEnvironmentConfiguration::getSMM() + "/shaders/defaultGL32.vert";
    std::string fragmentShader = ManagerEnvironmentConfiguration::getSMM() + "/shaders/defaultGL32PerVertexColor.frag";
    skin.setShader(GLSLShaderPerVertexColorGL32::New());
    worldLst->push_back(this->heightField);
}



void GLViewGaneshFinalProject::deleteGrid() {
    if (this->heightField) {
        worldLst->eraseViaWOptr(this->heightField);
    }
}

