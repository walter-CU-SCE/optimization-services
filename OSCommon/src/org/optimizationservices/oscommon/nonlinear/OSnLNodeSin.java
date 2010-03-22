/**
 * @(#)OSnLNodeSin 1.0 03/14/2004
 *
 * Copyright (c) 2004
 */
package org.optimizationservices.oscommon.nonlinear;

/**
 *
 * <P>The <code>OSnLNodeSin</code> class represents a sin node in an expression tree.
 * It extends the abstract OSnLNode class and implements its abstract methods such as
 * calculateFunction(double[]).
 * </p>
 *
 * </p>
 *
 * @author Robert Fourer, Jun Ma, Kipp Martin
 * @version 1.0, 03/14/2004
 * @see org.optimizationservices.oscommon.nonlinear.OSExpressionTree;
 * @see org.optimizationservices.oscommon.nonlinear.OSnLNode;
 * @since OS 1.0
 */
public class OSnLNodeSin extends OSnLNode{

	/**
	 * m_sSymbol holds the symbol that this nlNode looks in ASCII.
	 */
	protected String m_sSymbol = "sin";

	/**
	 * m_sSymbol holds the description of this nlNode
	 */
	protected String m_sDescription = "Sin function of a node operand";

	/**
	 * m_iSymbol holds the four digit integer that represents this nlNode, with the leftmost (first)
	 * integer representing the category and the right three digits representing a unique sequence number
	 * in the node category.
	 */
	protected int m_iSymbol = 3001;

	/**
	 * m_iType holds the integer representation of the type of this nlNode.
	 */
	protected int m_iType = 1;

	/**
	 * constructor
	 *
	 * </p>
	 *
	 * @param node holds the node operand of the sin function.
	 */
	protected OSnLNodeSin(OSnLNode node){
		m_mChildren = new OSnLNode[1];
		m_mChildren[0] = node;
	}//constructor

	/**
	 *
	 * Default Constructor
	 */
	protected OSnLNodeSin(){
		m_mChildren = new OSnLNode[1];
	}//constructor


	/**
	 * Calculate the result value of sin function given the current variable values.
	 *
	 * </p>
	 *
	 * @param x holds the values of the variables in a double array.
	 * @return the result value of sin function given the current variable values.
	 */
	protected double calculateFunction(double[] x){
		m_dFunctionValue = Math.sin(m_mChildren[0].calculateFunction(x));
		return m_dFunctionValue;
	}//calculateFunction

	/**
	 * Propogate the function derivatives w.r.t the nodes from the root to the node's children.
	 * This is mainly used for the the calculation of all derivatives at the same time to avoid
	 * redundancies, which is based on automatic differentiation.
	 *
	 * </p>
	 *
	 * @param rootNode holds root node of this current node.
	 * @param x holds the values of the variables in a double array.
	 * @see ExpressionTree#calculateDerivatives
	 */
	protected void propogateDerivatives(OSnLNode rootNode, double[] x){
		double dValue = m_mChildren[0].calculateFunction(x, true);
		m_mChildren[0].m_dNodeDerivative = m_dNodeDerivative * (Math.cos(dValue));
		m_mChildren[0].propogateDerivatives(rootNode, x);
	}//propogateDerivatives


	/**
	 * Calculate the result value of sin function derivative given the current variable values.
	 *
	 * </p>
	 *
	 * @param x holds the values of the variables in a double array.
	 * @param functionEvaluated holds whether the function has been evaluated.
	 * @return the result value of sin function derivative given the current variable values.
	 */
	protected double calculateDerivative(double[] x, int index, boolean functionEvaluated){
		double dDerivative = Math.cos(m_mChildren[0].calculateFunction(x, functionEvaluated)) * m_mChildren[0].calculateDerivative(x, index, functionEvaluated);
		return dDerivative;
	}//calculateDerivative

	/**
	 *
	 * @return the symbol of this nlNode in ASCII string.
	 */
	protected String getSymbol(){
		return m_sSymbol;
	}//getSymbol

	/**
	 *
	 * @return the symbol of this nlNode in integer.
	 */
	protected int getSymbolInteger(){
		return m_iSymbol;
	}//getSymbolInteger

	/**
	 *
	 * @return the type of this nlNode.
	 */
	protected int getType(){
		return m_iType;
	}//getType

	/**
	 *
	 * @return the description of this nlNode.
	 */
	protected String getDescription(){
		return m_sDescription;
	}//getDescription

}//class OSnLNodeSin