/*
 * =====================================================================================
 *
 *       Filename:  ConnFault.h
 *
 *    Description:  This class contains all the parameters associated with every node
 *                  in the circuit such as the inputs, outputs, type, line number.
 *
 *                  There are static members which keep track of total inputs, outputs
 *                  and lines in the entire circuit.
 *
 *        Version:  1.0
 *        Created:  12/02/2011
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aditya Shevade <aditya.shevade@gmail.com>,
 *                  Amey Marathe <marathe.amey@gmail.com>,
 *                  Samik Biswas <samiksb@gmail.com>,
 *                  Viraj Bhogle <viraj.bhogle@gmail.com>
 *
 * =====================================================================================
 */

#ifndef CONNFAULT_H_
#define CONNFAULT_H_



/*
 * =====================================================================================
 *        Struct:  ConnFault
 *  Description:  Each object of this class represents one line in the circuit.
 *                It can be of type Primary Input, Forward Branch, Primary Output or 
 *                a Logic Gate.
 *                
 *                There are a number of properties associated with each circuit line.
 *                All such properties are included below.
 * =====================================================================================
 */
struct ConnFault {
    int src;
	int dest;
	int pola;
};

#endif /* CONNFAULT_H_ */

