/**
 * @(#)OSnLNodeSimInput 1.0 03/14/2004
 *
 * Copyright (c) 2004
 */
package org.optimizationservices.oscommon.nonlinear;

import org.optimizationservices.oscommon.representationparser.OSsLReader;
import org.w3c.dom.Document;
import org.w3c.dom.Element;



/**
 *
 * <P>The <code>OSnLNodeSimInput</code> class represents a simulation input. A simulation input node
 * is used to point to an input element in the OSsL instance.
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
 * @see org.optimizationservices.oscommon.nonlinear.OSnLNodeSim;
 * @since OS 1.0
 */
public class OSnLNodeSimInput extends OSnLNode{

	/**
	 * m_sSymbol holds the symbol that this nlNode looks in ASCII.
	 */
	protected String m_sSymbol = "simInput";

	/**
	 * m_sSymbol holds the description of this nlNode
	 */
	protected String m_sDescription = "A simulation input node";

	/**
	 * m_iSymbol holds the four digit integer that represents this nlNode, with the leftmost (first)
	 * integer representing the category and the right three digits representing a unique sequence number
	 * in the node category.
	 */
	protected int m_iSymbol = 9004;

	/**
	 * m_iType holds the integer representation of the type of this nlNode.
	 */
	protected int m_iType = -1;

	/**
	 * m_sSimName holds the name of the simulation.
	 */
	protected String m_sSimName;

	/**
	 * m_sInputName holds the name of the input in a simulation.
	 */
	protected String m_sInputName;

	/**
	 * m_sURI holds the URI or the location of the simulation.
	 */
	protected String m_sURI = "";

	/**
	 * m_osslReader holds an OSsL reader object, from which we can get all the information of the OSsL instance
	 * wrapped inside this reader object.
	 */
	protected OSsLReader m_osslReader = null;

	/**
	 * constructor.
	 *
	 * </p>
	 *
	 * @param node holds the node operand as the simInput's input value.
	 * @param simName holds the name of the simulation.
	 * @param inputName holds the name of the input in a simulation.
	 */
	protected OSnLNodeSimInput(OSnLNode node, String simName, String inputName){
		m_mChildren = new OSnLNode[1];
		m_mChildren[0] = node;
		m_sSimName = simName;
		m_sInputName = inputName;
		m_sSymbol = "simInput:" + m_sSimName + ":" + m_sInputName;
	}//constructor

	/**
	 * constructor.
	 *
	 * </p>
	 *
	 * @param simName holds the name of the simulation.
	 * @param inputName holds the name of the input in a simulation.
	 */
	protected OSnLNodeSimInput(String simName, String inputName){
		m_sSimName = simName;
		m_sInputName = inputName;
		m_sSymbol = "simInput:" + m_sSimName + ":" + m_sInputName;
	}//constructor

	/**
	 * default constructor.
	 */
	protected OSnLNodeSimInput(){
	}//constructor

	/**
	 * Calculate the result value of a simulation input node given the current variable values.
	 *
	 * </p>
	 *
	 * @param x holds the values of the variables in a double array.
	 * @return the result value of the simulation input node given the current variable values.
	 */
	protected double calculateFunction(double[] x){
		if(m_mChildren != null && m_mChildren.length == 1){
			m_dFunctionValue = m_mChildren[0].calculateFunction(x);
			if(Double.isNaN(m_dFunctionValue) && m_mChildren[0].m_sFunctionValue != null){
				m_sFunctionValue = m_mChildren[0].m_sFunctionValue;
				try{
					m_dFunctionValue = Double.parseDouble(m_sFunctionValue);
					m_sFunctionValue = null;
				}
				catch(Exception e){
					m_dFunctionValue = Double.NaN;
				}
			}
		}
		else{
			m_sFunctionValue = m_osslReader.getInputByName(m_sInputName);;
			try{
				m_dFunctionValue = Double.parseDouble(m_sFunctionValue);
				m_sFunctionValue = null;
			}
			catch(Exception e){
				m_dFunctionValue = Double.NaN;
			}
		}
		return m_dFunctionValue;
	}//calculateFunction

	/**
	 * Calculate the function value given the current variable values.
	 * If the function has been evaluated over the current x values, the method will retrieve it.
	 *
	 * </p>
	 *
	 * @param x holds the values of the variables in a string array.
	 * @param functionEvaluated holds whether the function has been evaluated.
	 * @return the function derivative value given the current variable values in a string.
	 */
	protected String calculateFunction(String[] x, boolean functionEvaluated){
		if(m_mChildren != null && m_mChildren.length == 1){
			m_sFunctionValue = m_mChildren[0].calculateFunction(x, functionEvaluated);
		}
		else{
			m_sFunctionValue = m_osslReader.getInputByName(m_sInputName);
		}
		return m_sFunctionValue;
	}//calculateFunction - string based

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
		if(m_mChildren != null && m_mChildren.length == 1){
			m_mChildren[0].m_dNodeDerivative = m_dNodeDerivative;
			m_mChildren[0].propogateDerivatives(rootNode, x);
		}
	}//propogateDerivatives

	/**
	 * Calculate the result value of a simulation input node's derivative given the current variable values.
	 *
	 * </p>
	 *
	 * @param x holds the values of the variables in a double array.
	 * @param functionEvaluated holds whether the function has been evaluated.
	 * @return the result value of the simulation input node's derivative given the current variable values.
	 */
	protected double calculateDerivative(double[] x, int index, boolean functionEvaluated){
		double dDerivative;
		if(m_mChildren != null && m_mChildren.length == 1){
			try{
				dDerivative = m_mChildren[0].calculateDerivative(x, index, functionEvaluated);
			}
			catch(Exception e){
				dDerivative = 0;
			}
		}
		else{
			dDerivative = 0;
		}
		return dDerivative;
	}//calculateDerivative

	/**
	 *
	 * @param document holds the W3C DOM type document to create XML elements and attributes.
	 * It is the parent of the root element, e.g. the &lt;OSiL&gt; element in OSiL. It is used
	 * to create all the nodes in the DOM tree.
	 * @return the nonlinear expression starting with the current node in an XML DOM Tree.
	 */
	protected Element getDOMTree(Document document){
		Element eRoot = document.createElement("simInput");
		String sAttrValue = getSimName();
		if(sAttrValue != null && sAttrValue.length() > 0) eRoot.setAttribute("simName", sAttrValue);
		sAttrValue = getInputName();
		if(sAttrValue != null && sAttrValue.length() > 0) eRoot.setAttribute("simInput", sAttrValue);
		OSnLNode[] mChildren = getChildNodes();
		for(int i = 0; i < ((mChildren == null)?0:mChildren.length); i++){
			Element eChild = mChildren[i].getDOMTree(document);
			eRoot.appendChild(eChild);
		}
		return eRoot;
	}//getDOMTree

	/**
	 *
	 * @return the simulation name.
	 */
	protected String getSimName(){
		return m_sSimName;
	}//getSimName

	/**
	 * Set name of the simulation.
	 *
	 * </p>
	 *
	 * @param simName holds the name of the simulation.
	 */
	protected void setSimName(String simName){
		m_sSimName = simName;
		m_sSymbol = "simInput:" + m_sSimName + ":" + m_sInputName;
	}//setSimName

	/**
	 *
	 * @return the simulation input name.
	 */
	protected String getInputName(){
		return m_sInputName;
	}//getInputName

	/**
	 * Set name of the simulation input.
	 *
	 * </p>
	 *
	 * @param inputName holds the name of the simulation input.
	 */
	protected void setInputName(String inputName){
		m_sInputName = inputName;
		m_sSymbol = "simInput:" + m_sSimName + ":" + m_sInputName;
	}//setInputName

	/**
	 *
	 * @return the URI of the simulation.
	 */
	protected String getURI(){
		return m_sURI;
	}//getURI

	/**
	 * Set the URI of the simulation.
	 *
	 * </p>
	 *
	 * @param uri holds the uri of the simulation.
	 */
	protected void setURI(String uri){
		m_sURI = uri;
	}//setURI

	/**
	 *
	 * @return the OSsLReader of the simulation.
	 */
	protected OSsLReader getOSsLReader(){
		return m_osslReader;
	}//getOSsLReader

	/**
	 * Set the OSsLReader of the simulation.
	 *
	 * </p>
	 *
	 * @param osslReader holds an OSsL reader object, from which we can get all the information of the OSsL instance
	 * wrapped inside this reader object.
	 */
	protected void setOSsLReader(OSsLReader osslReader){
		m_osslReader = osslReader;
	}//setOSsLReader

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

}//class OSnLNodeSimInput