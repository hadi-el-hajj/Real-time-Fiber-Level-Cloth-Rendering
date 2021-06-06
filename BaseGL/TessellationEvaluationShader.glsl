#version 450 core // Minimal GL version support expected from the GPU

float pi = 3.14 ; 

//inspired by https://gamedev.stackexchange.com/questions/100614/gpu-friendly-bezier-storage-evaluation
vec3 bezier2(vec3 a, vec3 b, float t) {
    return mix(a, b, t);
}
vec3 bezier3(vec3 a, vec3 b, vec3 c, float t) {
    return mix(bezier2(a, b, t), bezier2(b, c, t), t);
}
vec3 bezier4(vec3 a, vec3 b, vec3 c, vec3 d, float t) {
    return mix(bezier3(a, b, c, t), bezier3(b, c, d, t), t);
}

layout(isolines) in;
in vec3 tcPos[];
in vec3 tcNormals[]; 
out vec4 fcolor; 
uniform mat4 projectionMat, modelViewMat; 
uniform sampler1D Texture; 

// VARIABLES ###################################################################################

float t = gl_TessCoord.x;
float v = gl_TessCoord.y; 

//parameters of the ellipse section of the ply
float ellipse_parameter_normal; 
float ellipse_parameter_base; 

//YARN VARIABLES
vec3 center_yarn;
vec3 normal_yarn; 
vec3 base_yarn; 

//PLY VARIABLES
vec3 normal_ply; 
vec3 base_ply; 
//variable used to determine the ply which is being worked on right now
//useful to set ply color and also ply angle
float ply; 
float radius_ply; 
vec3 center_ply; 
float angle_initial_ply; 
float angle_rotation_ply; 

//FIBER VARIABLES
float radius_fiber ;    
vec3 center_fiber; 
float angle_initial_fiber;
float angle_rotation_fiber; 
vec3 fiber ; 

// ###################################################################################

// ###################################################################################
// PART 1 : SIMPLE USE CASE

/* THIS PART IS AN EXAMPLE SIMPLIFIED CASE WHICH I USED TO SEE IF I UNDERSTAND THE CALCULATIONS CORRECTLY.
SECOND PART (BELOW AFTER END OF THIS PART) WILL TRY AND REPRODUCE THE METHDODOLOGY USED IN THE PAPER FOR
ANY PATCH OF 4 CONTROL POINTS */ 

/* The control points for this simplistic case are taken as aligned on the x axis so yarn normal and 
yarn base vectors are simple to evaluate (they are respectively the y and the z axis in this case) */ 

/*
void main() {
    //set the ply variable
    if(v>=0 && v*63 < 21){
        ply=1; 
        fcolor = vec4 (1.0,0.0,0.0, 1.0); 
    }
    else if(v*63 >=21 && v*63<42){
        ply=2; 
        fcolor = vec4 (0.0, 1.0 ,0.0 ,1.0);
    }
    else if(v*63 >=42){
        ply=3; 
        fcolor = vec4 (0.0, 0.0, 1.0 ,1.0); 
    }

    //YARN 
    center_yarn = t*tcPos[3] ;
    normal_yarn = vec3(0,1,0); 
    base_yarn = vec3(0,0,1); 

    //PLY 
    angle_initial_ply = ply/3*2*pi ; 
    angle_rotation_ply = 2*2*pi ; 
    radius_ply = 0.5 ; 
    center_ply = 0.5*radius_ply*
                (cos(angle_initial_ply+t*angle_rotation_ply)*normal_yarn 
                + sin(angle_initial_ply+t*angle_rotation_ply)*base_yarn) ; 

    //FIBER
    normal_ply = normalize(center_ply);   
    base_ply = -1*normalize(vec3(0,cos(pi/2)*normal_ply.y - sin(pi/2)*normal_ply.z,sin(pi/2)*normal_ply.y + cos(pi/2)*normal_ply.z));
    ellipse_parameter_normal = 1;
    ellipse_parameter_base   = 2;
    // radius is multiplied by a random shrinking factor accessed through the set 1D texture and set 
    // in the C++ code    
    radius_fiber = 0.1 * texelFetch(Texture,int(v*63),0).x;    //get value from 1D texture
    angle_initial_fiber = v*63/21*2*pi ; 
    angle_rotation_fiber = 2*2*pi; 
    center_fiber = radius_fiber*
                    (cos(angle_initial_fiber+t*angle_rotation_fiber)*normal_ply*ellipse_parameter_normal
                    + sin(angle_initial_fiber+t*angle_rotation_fiber)*base_ply*ellipse_parameter_base);
    fiber = center_yarn + center_ply +center_fiber ;

    gl_Position = projectionMat * modelViewMat * vec4(fiber, 1);
}
*/

// ###################################################################################

// PART 2: NORMAL IMPLEMENTATION (NON-ALIGNED PATCH CONTROL POINTS)
vec3 control_point_1 = tcPos[0]; 
vec3 control_point_2 = tcPos[1]; 
vec3 control_point_3 = tcPos[2]; 
vec3 control_point_4 = tcPos[3]; 

vec3 tangent_yarn;  //here we need to define the yarn tangent in order to get the normal of the curve

// vectors used for the Rotation Minimising Frame method (cf. later on)
vec3 past_point; 
vec3 past_normal ; 
vec3 past_tangent ; 
vec3 past_base ; 

/* 
 to calculate the tangent of the yarn curve, which in this case is a bezier curve, we simply derive
the cubic bezier curve equation with relation to t and use the derivative to calculate the tangent 
vector : B'(t) = 3(1-t)^2(P1-P0) + 6(1-t)t(P2-P1) + 3t^2(P3-P2) 
*/
vec3 bezier4_derivative(vec3 a, vec3 b, vec3 c, vec3 d, float t){
    vec3 derivative; 
    derivative = 3*(1-t)*(1-t)*(b-a) + 6*(1-t)*t*(c-b)+3*t*t*(d-c); 
    return derivative ; 
}




void main() {
    //set the ply variable
    if(v>=0 && v*63 < 21){
        ply=1; 
        fcolor = vec4 (1.0,0.0,0.0, 1.0); 
    }
    else if(v*63 >=21 && v*63<42){
        ply=2; 
        fcolor = vec4 (0.0, 1.0 ,0.0 ,1.0);
    }
    else if(v*63 >=42){
        ply=3; 
        fcolor = vec4 (0.0, 0.0, 1.0 ,1.0); 
    }

    //YARN 

    //center of the yarn is now given by the bezier curve defined by the four control points as cited in 
    //the paper
    center_yarn =  bezier4(control_point_1, control_point_2, control_point_3, control_point_4,t);

    //tangent is calculated by deriving the bezier curve
    tangent_yarn =  normalize(bezier4_derivative(control_point_1, control_point_2, control_point_3, control_point_4,t)); 

    //assuming yarn curve is in xy plane for example
    // normal_yarn = vec3(-yarn_tangent.y,yarn_tangent.x,0); 

    //with yarn curve in any plane 
    //calculations are inspired by what I found on https://pomax.github.io/bezierinfo/ especially the 
    //section 15, i.e. Working with 3D normals. This in turn is inspired by the paper:
    //"Computation of Rotation Minimizing Frames" (Wenping Wang, Bert Jüttler, Dayue Zheng, and Yang Liu, 2008)
    //In order to compute reliable normals, the only way I found was to rely on this mehtod. 
    
    // START ROTATION MINIMIZING FRAME METHOD ###################################

    //first, initialize the first frame
    if(t==0){
        past_point = tcPos[0]; 
        past_normal = normalize(tcNormals[0]);    
        normal_yarn = normalize(tcNormals[0]); 
        past_tangent = tangent_yarn;  
        past_base  = -cross(tangent_yarn, normal_yarn); 
        base_yarn = -cross(tangent_yarn, normal_yarn); 
    }

    if(t!=0){ // compute next frames 
        vec3 v = center_yarn - past_point ; 
        float c = dot(v,v); 
        vec3 riL = past_base - v*(2/c)*(dot(v,past_base)); 
        vec3 tiL = past_tangent - v*(2/c)*(dot(v,past_tangent));

        vec3 v2 = tangent_yarn - tiL ; 
        float c2 = dot(v2,v2); 

        base_yarn = riL-v2*(2/c2)*dot(v2,riL); 
        normal_yarn = cross(base_yarn,tangent_yarn) ; 

        past_point = center_yarn; 
        past_normal = normal_yarn; 
        past_base = base_yarn; 
        past_tangent = tangent_yarn; 
    }

    // END ######################################################################

    //PLY 
    angle_initial_ply = ply/3*2*pi ; 
    angle_rotation_ply = 2*2*pi ; 

    // RADIUS OF THE PLY 
    radius_ply = 0.004 ; 

    center_ply = 0.5*radius_ply*
                (cos(angle_initial_ply+t*angle_rotation_ply)*normal_yarn 
                + sin(angle_initial_ply+t*angle_rotation_ply)*base_yarn) ; 

    //FIBER

    ellipse_parameter_normal = 1;
    ellipse_parameter_base   = 2;
    /*radius is multiplied by a random shrinking factor accessed through the set 1D texture and set 
    in the C++ code*/ 
    radius_fiber = 0.002 * texelFetch(Texture,int(v*63),0).x;    //get value from 1D texture
    angle_initial_fiber = v*63/21*2*pi ; 
    angle_rotation_fiber = 2*2*pi; 

    // This time, we do not need any other method since the normal is given
    // to calculate the base vector, we simply calculate the cross product of the tangent and normal vectors

    // normal vector
    normal_ply = normalize(center_ply);  

    // base_ply = -1*normalize(vec3(0,cos(pi/2)*normal_ply.y - sin(pi/2)*normal_ply.z,sin(pi/2)*normal_ply.y + cos(pi/2)*normal_ply.z));
    base_ply = -cross(tangent_yarn, normal_ply);

    center_fiber = radius_fiber*
                    (cos(angle_initial_fiber+t*angle_rotation_fiber)*normal_ply*ellipse_parameter_normal
                    + sin(angle_initial_fiber+t*angle_rotation_fiber)*base_ply*ellipse_parameter_base);
    fiber = center_yarn + center_ply +center_fiber ;

    gl_Position = projectionMat * modelViewMat * vec4(fiber, 1);
}

// ###################################################################################
