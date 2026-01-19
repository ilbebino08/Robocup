#include "motori.h"

void Motori::init(){
    msx_vel = MSX_ZERO;
    mdx_vel = MDX_ZERO;
    mpo_vel = MPO_ZERO;

    mot_sx.attach(MSX_PIN);
    mot_dx.attach(MDX_PIN);
    mot_po.attach(MPO_PIN);

    // Delay per permittere ai servo di inizializzarsi correttamente
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
    
    // Calcola la componente di velocità in avanti (identica per entrambi i motori)
    short vel_avanti;
    if(vel >= 0){
        vel_avanti = map(vel, 0, 1023, 0, MSX_MAX - MSX_ZERO);
    } else {
        vel_avanti = -map(-vel, 0, 1023, 0, MSX_ZERO - MSX_MIN);
    }
    
    // Calcola la componente laterale per la sterzata (indipendente dalla velocità)
    short componente_laterale_sx, componente_laterale_dx;
    if(ang < 0){
        // Sterza a sinistra: sx rallenta/indietro, dx accelera/avanti
        componente_laterale_sx = map(-ang, 0, 1750, 0, -(MSX_ZERO - MSX_MIN));
        componente_laterale_dx = map(-ang, 0, 1750, 0, MSX_MAX - MSX_ZERO);
    } else if(ang > 0){
        // Sterza a destra: sx accelera/avanti, dx rallenta/indietro
        componente_laterale_sx = map(ang, 0, 1750, 0, MSX_MAX - MSX_ZERO);
        componente_laterale_dx = map(ang, 0, 1750, 0, -(MSX_ZERO - MSX_MIN));
    } else {
        componente_laterale_sx = 0;
        componente_laterale_dx = 0;
    }
    
    // Combina le componenti
    vel_sx = MSX_ZERO + vel_avanti + componente_laterale_sx;
    vel_dx = MDX_ZERO + vel_avanti + componente_laterale_dx;
    
    // Limita i valori ai range validi
    vel_sx = constrain(vel_sx, MSX_MIN, MSX_MAX);
    vel_dx = constrain(vel_dx, MDX_MIN, MDX_MAX);
    
    if(MSX_INV){
        vel_sx = MSX_ZERO - (vel_sx - MSX_ZERO);
    }
    

    if(MDX_INV){
        vel_dx = MDX_ZERO - (vel_dx - MDX_ZERO);
    }
    
    msx_vel = vel_sx;
    mdx_vel = vel_dx;

    // Calcola la velocità del motore posteriore come media dei due motori anteriori, considerando le inversioni
    short vel_sx_eff = MSX_INV ? (MSX_ZERO - (msx_vel - MSX_ZERO)) : msx_vel;
    short vel_dx_eff = MDX_INV ? (MDX_ZERO - (mdx_vel - MDX_ZERO)) : mdx_vel;
    mpo_vel = (vel_sx_eff + vel_dx_eff) / 2;

    if (MPO_INV) {
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