#pragma once
#define GLM_FORCE_RADIANS // forza l'utilizzo dei radianti per le funzioni trigonometriche e di rotazione
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // forza la mappatura della coordinata Z in [0,1]
#include <glm/glm.hpp> // per i vettori e le matrici base
#include <glm/gtc/matrix_transform.hpp> // per la matrice di proiezione e altre trasformazioni di matrici
#include <glm/gtx/rotate_vector.hpp> // per la rotazione di vettori
#include <glm/gtc/constants.hpp> // per le costanti matematiche come pi greco

// per la grafica 3D e il raytracing
#include <glm/gtc/type_ptr.hpp> 
#include <glm/gtx/intersect.hpp> 
#include <glm/gtx/norm.hpp>
