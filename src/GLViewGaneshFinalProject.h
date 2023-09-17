#pragma once

#include "GLView.h"
#include "WOGrid.h"
namespace Aftr
{
   class Camera;

/**
   \class GLViewGaneshFinalProject
   \author Scott Nykl 
   \brief A child of an abstract GLView. This class is the top-most manager of the module.

   Read \see GLView for important constructor and init information.

   \see GLView

    \{
*/

class GLViewGaneshFinalProject : public GLView
{
public:
    WOGrid* heightField;
    double lat ;  // New York City
    double lon ;
   static GLViewGaneshFinalProject* New( const std::vector< std::string >& outArgs );
   virtual ~GLViewGaneshFinalProject();
   virtual void updateWorld(); ///< Called once per frame
   virtual void loadMap(); ///< Called once at startup to build this module's scene
   virtual void createGrid(const std::vector<std::vector<double>> matrix);
   virtual void getChunk(double lat, double lon, std::string type);
   virtual void onResizeWindow(GLsizei width, GLsizei height);
   virtual void onMouseDown(const SDL_MouseButtonEvent& e);
   virtual void onMouseUp(const SDL_MouseButtonEvent& e);
   virtual void onMouseMove(const SDL_MouseMotionEvent& e);
   virtual void onKeyDown(const SDL_KeyboardEvent& key);
   virtual void onKeyUp(const SDL_KeyboardEvent& key);
   virtual void deleteGrid();

protected:
   GLViewGaneshFinalProject( const std::vector< std::string >& args );
   virtual void onCreate();   
};



} //namespace Aftr
