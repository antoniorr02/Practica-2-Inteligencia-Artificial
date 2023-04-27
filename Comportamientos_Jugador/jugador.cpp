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
 * Devuelve el estado que se genera si el agente puede avanzar.
 * Si no puede avanzar, se devuelve como salida el mismo estado de entrada.
*/
stateN1 apply_1 (const Action &a, const stateN1 &st, const vector<vector<unsigned char>> &mapa) {
	stateN1 st_result = st;
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
		case actSON_FORWARD:
			sig_ubicacion = NextCasilla(st.sonambulo);
			if (CasillaTransitable(sig_ubicacion, mapa) && !(sig_ubicacion.f == st.jugador.f && sig_ubicacion.c == st.jugador.c)) {
				st_result.sonambulo = sig_ubicacion;
			}
		break;
		case actTURN_SL:
			st_result.sonambulo.brujula = static_cast<Orientacion>((st_result.sonambulo.brujula+7)%8);
		break;
		case actTURN_SR:
			st_result.sonambulo.brujula = static_cast<Orientacion>((st_result.sonambulo.brujula+1)%8);
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
				cst.jugador.brujula = (Orientacion)((cst.jugador.brujula+6)%8);
			break;
			case actTURN_R:
				cst.jugador.brujula = (Orientacion)((cst.jugador.brujula+2)%8);
			break;
			case actSON_FORWARD:
				cst.sonambulo = NextCasilla(cst.sonambulo);
				mapaConPlan[cst.sonambulo.f][cst.sonambulo.c] = 2;
			break;
			case actSON_TURN_SL:
				cst.sonambulo.brujula = (Orientacion)((cst.sonambulo.brujula+7)%8);
			break;
			case actSON_TURN_SR:
				cst.sonambulo.brujula = (Orientacion)((cst.sonambulo.brujula+1)%8);
			break;
		}
		it++;
	}
}

/**
 * Búsqueda en anchura para el jugador (nivel 0).
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
			if (explored.find(child_turnl) == explored.end()) {
				child_turnl.secuencia.push_back(actTURN_L);
				frontier.push_back(child_turnl);
			}
			// Generar hijo actTURN_R
			nodeNO child_turnr = current_node;
			child_turnr.st = apply(actTURN_R, current_node.st, mapa);
			if (explored.find(child_turnr) == explored.end()) {
				child_turnr.secuencia.push_back(actTURN_R);
				frontier.push_back(child_turnr);
			}
		}

		if (!SolutionFound && !frontier.empty()) {
			current_node = frontier.front();
			while (!frontier.empty() && explored.find(current_node) != explored.end()) {
				frontier.pop_front();
				current_node = frontier.front();
			}

		}
	}

	if(SolutionFound) {
		plan = current_node.secuencia;
	}

	return plan;
}


/**
 * Comprueba si el sonámbulo está en el campo de visión del jugador. 
*/
bool SonambuloAlaVista(const stateN1 &st) {
	bool encontrado = false;
	bool pos1, pos2, pos3, pos4, pos5, pos6, pos7, pos8, pos9, pos10, pos11, pos12, pos13, pos14, pos15;
	switch (st.jugador.brujula) {
		case norte:
			pos1 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos2 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos3 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos4 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos5 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos6 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos7 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos8 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos9 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos10 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos11 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos12 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos13 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos14 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos15 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
		break;
		case noreste:
			pos1 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos2 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos3 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos4 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos5 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos6 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos7 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos8 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos9 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos10 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos11 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos12 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos13 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos14 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos15 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
		break;
		case este:
			pos1 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos2 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos3 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos4 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos5 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos6 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos7 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos8 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos9 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos10 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos11 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos12 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos13 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos14 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos15 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
		break;
		case sureste:
			pos1 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos2 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos3 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos4 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos5 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos6 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos7 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos8 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos9 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos10 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos11 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos12 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos13 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos14 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos15 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
		break;
		case sur:
			pos1 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos2 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos3 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos4 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos5 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos6 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos7 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos8 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos9 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c+3 == st.sonambulo.c));
			pos10 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c+2 == st.sonambulo.c));
			pos11 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c+1 == st.sonambulo.c));
			pos12 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos13 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos14 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos15 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
		break;
		case suroeste:
			pos1 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos2 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos3 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos4 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos5 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos6 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos7 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos8 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos9 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos10 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos11 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos12 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos13 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos14 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos15 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
		break;
		case oeste:
			pos1 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos2 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos3 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos4 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos5 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos6 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos7 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos8 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos9 = ((st.jugador.f+3 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos10 = ((st.jugador.f+2 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos11 = ((st.jugador.f+1 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos12 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos13 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos14 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos15 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
		break;
		case noroeste:
			pos1 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos2 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos3 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos4 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos5 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos6 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos7 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos8 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
			pos9 = ((st.jugador.f == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos10 = ((st.jugador.f-1 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos11 = ((st.jugador.f-2 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos12 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c-3 == st.sonambulo.c));
			pos13 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c-2 == st.sonambulo.c));
			pos14 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c-1 == st.sonambulo.c));
			pos15 = ((st.jugador.f-3 == st.sonambulo.f) && (st.jugador.c == st.sonambulo.c));
		break;
	}
	if (pos1 || pos2 || pos3 || pos4 || pos5 || pos6 || pos7 || pos8 || pos9 || pos10 || pos11 || pos12 || pos13 || pos14 || pos15) {
		encontrado = true;
	}
	
	return encontrado;
}

/**
 * Anchura para el jugador y el sonámbulo. (Nivel 1)
 * NOTA: La función SonambuloAlaVista comprueba que el sonámbulo esté en el campo de visión
 * del jugador, para ello hago un switch y según la orientación del jugador miro si alguna de 
 * las posiciones del campo de visión coincide con la del sonámbulo.
*/
list<Action> AnchuraAmbos(const Sensores sensores, const stateN1 &inicio, const ubicacion &final, const vector<vector<unsigned char>> &mapa) {
	nodeN1 current_node;
	current_node.st = inicio;
	list<nodeN1> frontier;
	set<nodeN1> explored;
	list<Action> plan;
	bool SolutionFound =  (current_node.st.sonambulo.f == final.f and current_node.st.sonambulo.c == final.c);
	bool encontrado = false;

	frontier.push_back(current_node);

	while (!frontier.empty() && !SolutionFound) {
		frontier.pop_front();
		explored.insert(current_node);
		encontrado = SonambuloAlaVista(current_node.st);

		nodeN1 child_SON_forward = current_node;
		child_SON_forward.st = apply_1(actSON_FORWARD, current_node.st, mapa);
		if (child_SON_forward.st.sonambulo.f == final.f and child_SON_forward.st.sonambulo.c == final.c) {
			child_SON_forward.secuencia.push_back(actSON_FORWARD); 
			current_node = child_SON_forward;
			SolutionFound = true;
		}else
		

		if (!SolutionFound) {
			
			// Generar hijo actFORWARD
			nodeN1 child_forward = current_node;
			child_forward.st = apply_1(actFORWARD, current_node.st, mapa);
			if (explored.find(child_forward) == explored.end()) {
				child_forward.secuencia.push_back(actFORWARD);
				frontier.push_back(child_forward);
			}
			// Generar hijo actTURN_L
			nodeN1 child_turnl = current_node; 
			child_turnl.st = apply_1(actTURN_L, current_node.st, mapa);
			if (explored.find(child_turnl) == explored.end()) {
				child_turnl.secuencia.push_back(actTURN_L);
				frontier.push_back(child_turnl);
			}
			// Generar hijo actTURN_R
			nodeN1 child_turnr = current_node;
			child_turnr.st = apply_1(actTURN_R, current_node.st, mapa);
			if (explored.find(child_turnr) == explored.end()) {
				child_turnr.secuencia.push_back(actTURN_R);
				frontier.push_back(child_turnr);
			}
			
			if (encontrado) {
				// Generar hijo actSON_FORWARD
				if (explored.find(child_SON_forward) == explored.end()) {
					child_SON_forward.secuencia.push_back(actSON_FORWARD);
					frontier.push_back(child_SON_forward);
				}
				// Generar hijo actTURN_SL
				nodeN1 child_turnsl = current_node; 
				child_turnsl.st = apply_1(actSON_TURN_SL, current_node.st, mapa);
				if (explored.find(child_turnsl) == explored.end()) {
					child_turnsl.secuencia.push_back(actSON_TURN_SL);
					frontier.push_back(child_turnsl);
				}
				// Generar hijo actTURN_SR
				nodeN1 child_turnsr = current_node;
				child_turnsr.st = apply_1(actSON_TURN_SR, current_node.st, mapa);
				if (explored.find(child_turnsr) == explored.end()) {
					child_turnsr.secuencia.push_back(actSON_TURN_SR);
					frontier.push_back(child_turnsr);
				}
			}
		}

		if (!SolutionFound && !frontier.empty()) {
			current_node = frontier.front();
			while (!frontier.empty() && explored.find(current_node) != explored.end()) {
				frontier.pop_front();
				if(!frontier.empty())
					current_node = frontier.front();
			}

		}		
	}

	if (SolutionFound) {
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
			goal.f = sensores.destinoF;
			goal.c = sensores.destinoC;

			switch (sensores.nivel){
				case 0:
					c_state.jugador.f = sensores.posF;
					c_state.jugador.c = sensores.posC;
					c_state.jugador.brujula = sensores.sentido;
					c_state.sonambulo.f = sensores.SONposF;
					c_state.sonambulo.c = sensores.SONposC;
					c_state.sonambulo.brujula = sensores.SONsentido;
					plan = AnchuraSoloJugador(c_state, goal, mapaResultado);
 				break;
				case 1:
					c_state_1.jugador.f = sensores.posF;
					c_state_1.jugador.c = sensores.posC;
					c_state_1.jugador.brujula = sensores.sentido;
					c_state_1.sonambulo.f = sensores.SONposF;
					c_state_1.sonambulo.c = sensores.SONposC;
					c_state_1.sonambulo.brujula = sensores.SONsentido;
					plan = AnchuraAmbos(sensores, c_state_1, goal, mapaResultado);
				break;
				case 2:
					cout << "Pendiente de implementar el nivel 2\n";
				break;
				case 3:
					cout << "Pendiente de implementar el nivel 3\n";
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
