#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"

#include <iostream>
#include <cmath>
#include <set>
#include <stack>

/**
 * Devuelve si una ubicación en el mapa es transitable para el agente.
*/
bool CasillaTransitable (const ubicacion &x, const vector<vector<unsigned char>> &mapa) {
	return (mapa[x.f][x.c] != 'P' && mapa[x.f][x.c] != 'M');
}

/**
 * Devuelve la ubicación siguiente según el avance del agente.
*/
ubicacion NextCasilla (const ubicacion &pos) {
	ubicacion next = pos;
	switch (pos.brujula) {
		case norte:
			next.f = pos.f - 1;
		break;
		case noreste:
			next.f = pos.f - 1;
			next.c = pos.c + 1;
		break;
		case este:
			next.c = pos.c + 1;
		break;
		case sureste:
			next.f = pos.f + 1;
			next.c = pos.c + 1;
		break;
		case sur:
			next.f = pos.f + 1;
		break;
		case suroeste:
			next.f = pos.f + 1;
			next.c = pos.c - 1;
		break;
		case oeste:
			next.c = pos.c - 1;
		break;
		case noroeste:
			next.f = pos.f - 1;
			next.c = pos.c - 1;
		break;
	}
	return next;
}

/**
 * Devuelve el estado que se genera si el agente puede avanzar.
 * Si no puede avanzar, se devuelve como salida el mismo estado de entrada.
*/
stateNO apply (const Action &a, const stateNO &st, const vector<vector<unsigned char>> &mapa) {
	stateNO st_result = st;
	ubicacion sig_ubicacion;
	switch (a) {
		case actFORWARD:
			sig_ubicacion = NextCasilla(st.jugador);
			if (CasillaTransitable(sig_ubicacion, mapa) && !(sig_ubicacion.f == st.sonambulo.f && sig_ubicacion.c == st.sonambulo.c)) {
				st_result.jugador = sig_ubicacion;
			}
		break;
		case actTURN_L:
			st_result.jugador.brujula = static_cast<Orientacion>((st_result.jugador.brujula+6)%8);
		break;
		case actTURN_R:
			st_result.jugador.brujula = static_cast<Orientacion>((st_result.jugador.brujula+2)%8);
		break;
	}
	return st_result;
}

/**
 * Pone  a cero todos los elementos de una matriz
*/
void AnularMatriz(vector<vector<unsigned char>> &matriz) {
	for (int i = 0; i < matriz.size(); i++) {
		for (int j = 0; j < matriz.size(); j++) {
			matriz[i][j] = 0;
		}
	}
}

void ComportamientoJugador::VisualizaPlan (const stateNO &st, const list<Action> &plan) {
	AnularMatriz(mapaConPlan);
	stateNO cst = st;

	auto it = plan.begin();
	while (it != plan.end()) {
		switch (*it){
			case actFORWARD:
				cst.jugador = NextCasilla(cst.jugador);
				mapaConPlan[cst.jugador.f][cst.jugador.c] = 1;
			break;
			case actTURN_L:
				cst.jugador.brujula = static_cast<Orientacion>((cst.jugador.brujula+6)%8);
			break;
			case actTURN_R:
				cst.jugador.brujula = static_cast<Orientacion>((cst.jugador.brujula+2)%8);
			break;
			case actSON_FORWARD:
				cst.sonambulo = NextCasilla(cst.sonambulo);
				mapaConPlan[cst.sonambulo.f][cst.sonambulo.c] = 2;
			break;
			case actSON_TURN_SL:
				cst.sonambulo.brujula = static_cast<Orientacion>((cst.sonambulo.brujula+7)%8);
			break;
			case actSON_TURN_SR:
				cst.sonambulo.brujula = static_cast<Orientacion>((cst.sonambulo.brujula+1)%8);
			break;
		}
	}
}

/**
 * Búsqueda en anchura.
*/
list<Action> AnchuraSoloJugador(const stateNO &inicio, const ubicacion &final, const vector<vector<unsigned char>> &mapa) {
	nodeNO current_node;
	current_node.st = inicio;
	list<nodeNO> frontier;
	set<nodeNO> explored;
	list<Action> plan;
	bool SolutionFound = (current_node.st.jugador.f == final.f and current_node.st.jugador.c == final.c);
	frontier.push_back(current_node);

	while (!frontier.empty() and !SolutionFound) {
		frontier.pop_front();
		explored.insert(current_node);

		// Generar hijo actFORWARD
		nodeNO child_forward = current_node;
		child_forward.st = apply(actFORWARD, current_node.st, mapa);
		if (child_forward.st.jugador.f == final.f and child_forward.st.jugador.c == final.c) {
			child_forward.secuencia.push_back(actFORWARD); 
			current_node = child_forward;
			SolutionFound = true;
		} else if (explored.find(child_forward) == explored.end()) {
			child_forward.secuencia.push_back(actFORWARD);
			frontier.push_back(child_forward);
		}

		if (!SolutionFound) {
			// Generar hijo actTURN_L
			nodeNO child_turnl = current_node; 
			child_turnl.st = apply(actTURN_L, current_node.st, mapa);
			if (explored.find(child_forward) == explored.end()) {
				child_turnl.secuencia.push_back(actTURN_L);
				frontier.push_back(child_turnl);
			}
			// Generar hijo actTURN_R
			nodeNO child_turnr = current_node;
			child_turnr.st = apply(actTURN_R, current_node.st, mapa);
			if (explored.find(child_forward) == explored.end()) {
				child_turnr.secuencia.push_back(actTURN_R);
				frontier.push_back(child_turnr);
			}
		}

		if (!SolutionFound)
		{
			current_node = frontier.front();
		}
	}

	if(SolutionFound) {
		plan = current_node.secuencia;
	}

	return plan;
}


// Este es el método principal que se piden en la practica.
// Tiene como entrada la información de los sensores y devuelve la acción a realizar.
// Para ver los distintos sensores mirar fichero "comportamiento.hpp"
Action ComportamientoJugador::think(Sensores sensores)
{
	Action accion = actIDLE;

	// Incluir aquí el comportamiento del agente jugador
	if (sensores.nivel != 4) {
		if (!hayPlan) {
			cout << "Calculando un nuevo plan\n";
			c_state.jugador.f = sensores.posF;
			c_state.jugador.c = sensores.posC;
			c_state.jugador.brujula = sensores.sentido;
			c_state.sonambulo.f = sensores.SONposF;
			c_state.sonambulo.c = sensores.SONposC;
			c_state.sonambulo.brujula = sensores.SONsentido;
			goal.f = sensores.destinoF;
			goal.c = sensores.destinoC;
			switch (sensores.nivel){
				case 0:
					plan = AnchuraSoloJugador(c_state, goal, mapaResultado);
				break;
				case 1:
					cout << "Pendiente de implementar el nivel 1\n";
					break; 
				break;
				case 2:
					cout << "Pendiente de implementar el nivel 2\n";
					break; 
				break;
				case 3:
					cout << "Pendiente de implementar el nivel 3\n";
					break; 
				break;
			}
			if (plan.size() > 0) {
				VisualizaPlan(c_state, plan);
				hayPlan = true;
			}
		} 
		if (hayPlan && plan.size() > 0) {
			cout << "Ejecutando la siguiente acción del plan\n";
			accion = plan.front();
			plan.pop_front();
		}
		if (plan.size() == 0) {
			cout << "Se completó el plan\n";
			hayPlan = false;
		}
	} else {
		// AQUI NIVEL 4
	}
	return accion;
}


int ComportamientoJugador::interact(Action accion, int valor)
{
	return false;
}
