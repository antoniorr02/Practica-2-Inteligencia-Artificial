#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include "comportamientos/comportamiento.hpp"

#include <list>
#include <queue>

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

struct stateN2 {
  ubicacion jugador;
  ubicacion sonambulo;
  bool bikini_jugador, zapatillas_jugador;
  int costeTotal; // Coste acumulado para llegar a un nodo.

  bool operator== (const stateN2 &x) const {
    if (jugador == x.jugador && sonambulo.f == x.sonambulo.f && sonambulo.c == x.sonambulo.c && bikini_jugador == x.bikini_jugador && zapatillas_jugador == x.zapatillas_jugador && costeTotal == x.costeTotal) {
      return true;
    } else {
      return false;
    }
  }

  bool operator<(const stateN2 &n) const {
    if (jugador.f < n.jugador.f)
      return true;
    else if (jugador.f == n.jugador.f && jugador.c < n.jugador.c)
      return true;
    else if (jugador.f == n.jugador.f && jugador.c == n.jugador.c && jugador.brujula < n.jugador.brujula)
      return true;
    else if (jugador.f == n.jugador.f && jugador.c == n.jugador.c && jugador.brujula == n.jugador.brujula && zapatillas_jugador < n.zapatillas_jugador)
      return true;
    else if (jugador.f == n.jugador.f && jugador.c == n.jugador.c && jugador.brujula == n.jugador.brujula && zapatillas_jugador == n.zapatillas_jugador && bikini_jugador < n.bikini_jugador)
      return true;
    else
      return false;
  }
};

struct nodeN2 {
  stateN2 st;
  list<Action> secuencia;

  bool operator==(const nodeN2 &n) const {
    return (st == n.st);
  }

  bool operator<(const nodeN2 &n) const {
    if (st.costeTotal > n.st.costeTotal)
      return true;
    else
      return false;
  }
};

struct stateN3 {
  ubicacion jugador;
  ubicacion sonambulo;
  bool bikini_jugador, zapatillas_jugador, bikini_sonambulo, zapatillas_sonambulo;
  int costeTotal; // Coste acumulado para llegar a un nodo.
  int heuristica; // Heurística

  bool operator== (const stateN3 &x) const {
    if (jugador == x.jugador && sonambulo == x.sonambulo && bikini_jugador == x.bikini_jugador && zapatillas_jugador == x.zapatillas_jugador && bikini_sonambulo == x.bikini_sonambulo && zapatillas_sonambulo == x.zapatillas_sonambulo && costeTotal == x.costeTotal) {
      return true;
    } else {
      return false;
    }
  }

  bool operator<(const stateN3 &n) const {
    if (jugador.f < n.jugador.f)
      return true;
    else if (jugador.f == n.jugador.f && jugador.c < n.jugador.c)
      return true;
    else if (jugador.f == n.jugador.f && jugador.c == n.jugador.c && jugador.brujula < n.jugador.brujula)
      return true;
    else if (jugador.f == n.jugador.f && jugador.c == n.jugador.c && jugador.brujula == n.jugador.brujula && sonambulo.f < n.sonambulo.f)
      return true;
    else if (jugador.f == n.jugador.f && jugador.c == n.jugador.c && jugador.brujula == n.jugador.brujula && sonambulo.f == n.sonambulo.f  && sonambulo.c < n.sonambulo.c)
      return true;
    else if (jugador.f == n.jugador.f && jugador.c == n.jugador.c && jugador.brujula == n.jugador.brujula && sonambulo.f == n.sonambulo.f && sonambulo.c == n.sonambulo.c && sonambulo.brujula < n.sonambulo.brujula)
      return true;
    else if (jugador.f == n.jugador.f && jugador.c == n.jugador.c && jugador.brujula == n.jugador.brujula && sonambulo.f == n.sonambulo.f && sonambulo.c == n.sonambulo.c && sonambulo.brujula == n.sonambulo.brujula && zapatillas_jugador < n.zapatillas_jugador) 
      return true;
    else if (jugador.f == n.jugador.f && jugador.c == n.jugador.c && jugador.brujula == n.jugador.brujula && sonambulo.f == n.sonambulo.f && sonambulo.c == n.sonambulo.c && sonambulo.brujula == n.sonambulo.brujula && zapatillas_jugador == n.zapatillas_jugador && bikini_jugador < n.bikini_jugador) 
      return true;
    else if (jugador.f == n.jugador.f && jugador.c == n.jugador.c && jugador.brujula == n.jugador.brujula && sonambulo.f == n.sonambulo.f && sonambulo.c == n.sonambulo.c && sonambulo.brujula == n.sonambulo.brujula && zapatillas_jugador == n.zapatillas_jugador && bikini_jugador == n.bikini_jugador && zapatillas_sonambulo < n.zapatillas_sonambulo) 
      return true;
    else if (jugador.f == n.jugador.f && jugador.c == n.jugador.c && jugador.brujula == n.jugador.brujula && sonambulo.f == n.sonambulo.f && sonambulo.c == n.sonambulo.c && sonambulo.brujula == n.sonambulo.brujula && zapatillas_jugador == n.zapatillas_jugador && bikini_jugador == n.bikini_jugador && zapatillas_sonambulo == n.zapatillas_sonambulo && bikini_sonambulo < n.bikini_sonambulo) 
      return true;
    else
      return false;
  }
};

struct nodeN3 {
  stateN3 st;
  list<Action> secuencia;

  bool operator==(const nodeN3 &n) const {
    return (st == n.st);
  }

  bool operator<(const nodeN3 &n) const {
    if ((st.costeTotal + st.heuristica) > (n.st.costeTotal + n.st.heuristica))
      return true;
    else
      return false;
  }
};

class ComportamientoJugador : public Comportamiento {
  public:
    ComportamientoJugador(unsigned int size) : Comportamiento(size) {
      // Inicializar Variables de Estado
      hayPlan = false;
      ubicado = false;
      primerEncuentroSonambulo = false;
      murosIniciales = false;
      c_state_4.bikini_jugador = false;
			c_state_4.zapatillas_jugador = false;
			c_state_4.costeTotal = 0;
			c_state_4.heuristica = 0;
    }
    ComportamientoJugador(std::vector< std::vector< unsigned char> > mapaR) : Comportamiento(mapaR) {
      // Inicializar Variables de Estado
      hayPlan = false;
      ubicado = false;
      primerEncuentroSonambulo = false;
      c_state_4.bikini_jugador = false;
			c_state_4.zapatillas_jugador = false;
			c_state_4.costeTotal = 0;
			c_state_4.heuristica = 0;
      last_action = actIDLE;
    }
    ComportamientoJugador(const ComportamientoJugador & comport) : Comportamiento(comport){}
    ~ComportamientoJugador(){}
    void VisualizaPlan0(const stateNO &st, const list<Action> &plan);
    void VisualizaPlan1(const stateN1 &st, const list<Action> &plan);
    void VisualizaPlan2(const stateN2 &st, const list<Action> &plan);
    void VisualizaPlan3(const stateN3 &st, const list<Action> &plan);

    Action think(Sensores sensores);
    int interact(Action accion, int valor);


  private:
    // Declarar Variables de Estado
    list<Action> plan;
    bool hayPlan;
    stateNO c_state;
    stateN1 c_state_1;
    stateN2 c_state_2;
    stateN3 c_state_3, c_state_4;
    ubicacion goal;
    bool ubicado, primerEncuentroSonambulo, murosIniciales; // Para nivel 4.
    Action last_action;


};

#endif
