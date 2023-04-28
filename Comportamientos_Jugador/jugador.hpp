#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include "comportamientos/comportamiento.hpp"

#include <list>

struct stateNO {
  ubicacion jugador;
  ubicacion sonambulo;

  bool operator== (const stateNO &x) const {
    if (jugador == x.jugador && sonambulo.f == x.sonambulo.f && sonambulo.c == x.sonambulo.c) {
      return true;
    } else {
      return false;
    }
  }
};

struct nodeNO {
  stateNO st;
  list<Action> secuencia;

  bool operator==(const nodeNO &n) const {
    return (st == n.st);
  }

  bool operator<(const nodeNO &n) const {
    if (st.jugador.f < n.st.jugador.f)
      return true;
    else if (st.jugador.f == n.st.jugador.f && st.jugador.c < n.st.jugador.c)
      return true;
    else if (st.jugador.f == n.st.jugador.f && st.jugador.c == n.st.jugador.c && st.jugador.brujula < n.st.jugador.brujula)
      return true;
    else
      return false;
  }
};

struct stateN1 {
  ubicacion jugador;
  ubicacion sonambulo;

  bool operator== (const stateN1 &x) const {
    if (jugador == x.jugador && sonambulo == x.sonambulo) {
      return true;
    } else {
      return false;
    }
  }
};

struct nodeN1 {
  stateN1 st;
  list<Action> secuencia;

  bool operator==(const nodeN1 &n) const {
    return (st == n.st);
  }

  bool operator<(const nodeN1 &n) const {
    if (st.jugador.f < n.st.jugador.f)
      return true;
    else if (st.jugador.f == n.st.jugador.f && st.jugador.c < n.st.jugador.c)
      return true;
    else if (st.jugador.f == n.st.jugador.f && st.jugador.c == n.st.jugador.c && st.jugador.brujula < n.st.jugador.brujula)
      return true;
    else if (st.jugador.f == n.st.jugador.f && st.jugador.c == n.st.jugador.c && st.jugador.brujula == n.st.jugador.brujula && st.sonambulo.f < n.st.sonambulo.f)
      return true;
    else if (st.jugador.f == n.st.jugador.f && st.jugador.c == n.st.jugador.c && st.jugador.brujula == n.st.jugador.brujula && st.sonambulo.f == n.st.sonambulo.f  && st.sonambulo.c < n.st.sonambulo.c)
      return true;
    else if (st.jugador.f == n.st.jugador.f && st.jugador.c == n.st.jugador.c && st.jugador.brujula == n.st.jugador.brujula && st.sonambulo.f == n.st.sonambulo.f && st.sonambulo.c == n.st.sonambulo.c && st.sonambulo.brujula < n.st.sonambulo.brujula)
      return true;
    else
      return false;

  }
};

class ComportamientoJugador : public Comportamiento {
  public:
    ComportamientoJugador(unsigned int size) : Comportamiento(size) {
      // Inicializar Variables de Estado
    }
    ComportamientoJugador(std::vector< std::vector< unsigned char> > mapaR) : Comportamiento(mapaR) {
      // Inicializar Variables de Estado
      hayPlan = false;
    }
    ComportamientoJugador(const ComportamientoJugador & comport) : Comportamiento(comport){}
    ~ComportamientoJugador(){}
    void VisualizaPlan0(const stateNO &st, const list<Action> &plan);
    void VisualizaPlan1(const stateN1 &st, const list<Action> &plan);

    Action think(Sensores sensores);
    int interact(Action accion, int valor);


  private:
    // Declarar Variables de Estado
    list<Action> plan;
    bool hayPlan;
    stateNO c_state;
    stateN1 c_state_1;
    ubicacion goal;


};

#endif
