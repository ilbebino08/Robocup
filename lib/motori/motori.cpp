#include "motori.h"

void Motori::init(){
    msx_vel = MSX_ZERO;
    mdx_vel = MDX_ZERO;
    mpo_vel = MPO_ZERO;

    mot_sx.attach(MSX_PIN);
    mot_dx.attach(MDX_PIN);
    mot_po.attach(MPO_PIN);

    // Delay per permitere ai servo di inizializzarsi correttamente
    delay(50);

    mot_sx.writeMicroseconds(msx_vel);
    mot_dx.writeMicroseconds(mdx_vel);
    mot_po.writeMicroseconds(mpo_vel);
    
    // Altro delay per far raggiungere la posizione di stop
    delay(100);
}

void Motori::muovi(short vel, short ang){
    vel=constrain(vel, -1023, 1023);
    ang=constrain(ang, -1750, 1750);
    
    short vel_sx, vel_dx;
    
    if(vel==0){
        if(ang < 0){
            vel_sx = map(ang, 0, -1750, MSX_ZEROMAX, MSX_MAX);
            vel_dx = map(ang, 0, -1750, MDX_ZEROMIN, MDX_MIN);
        }
        else{
            vel_sx = map(ang, 0, 1750, MSX_ZEROMIN, MSX_MIN);
            vel_dx = map(ang, 0, 1750, MDX_ZEROMAX, MDX_MAX);
        }
    }
    else if(vel>0){
        vel_sx = map(vel, 0, 1023, MSX_ZEROMIN, MSX_MAX);
        vel_dx = map(vel, 0, 1023, MDX_ZEROMIN, MDX_MAX);
        
        if(ang < 0){
            vel_sx = map(ang, 0, -1750, vel_sx, MSX_ZEROMIN);
        }
        else{
            vel_dx = map(ang, 0, 1750, vel_dx, MDX_ZEROMIN);
        }
    }
    else{
        vel_sx = map(-vel, 0, 1023, MSX_ZEROMAX, MSX_MIN);
        vel_dx = map(-vel, 0, 1023, MDX_ZEROMAX, MDX_MIN);
        
        if(ang < 0){
            vel_sx = map(ang, 0, -1750, vel_sx, MSX_ZEROMAX);
        }
        else{
            vel_dx = map(ang, 0, 1750, vel_dx, MDX_ZEROMAX);
        }
    }
    
    if(MSX_INV){
        vel_sx = MSX_ZERO - (vel_sx - MSX_ZERO);
    }
    

    if(MDX_INV){
        vel_dx = MDX_ZERO - (vel_dx - MDX_ZERO);
    }
    
    msx_vel = vel_sx;
    mdx_vel = vel_dx;
    
    // Il motore posteriore deve muoversi in base alla velocità, non ai motori frontali
    if(vel >= 0){
        mpo_vel = map(vel, 0, 1023, MPO_ZEROMIN, MPO_MAX);
    } else {
        mpo_vel = map(-vel, 0, 1023, MPO_ZEROMAX, MPO_MIN);
    }

    if(MPO_INV){
        mpo_vel = MPO_ZERO - (mpo_vel - MPO_ZERO);
    }

    mot_sx.writeMicroseconds(msx_vel);
    mot_dx.writeMicroseconds(mdx_vel);
    mot_po.writeMicroseconds(mpo_vel);
}

void Motori::stop(){
    mot_sx.writeMicroseconds(MSX_ZERO);
    mot_dx.writeMicroseconds(MDX_ZERO);
    mot_po.writeMicroseconds(MPO_ZERO);
}