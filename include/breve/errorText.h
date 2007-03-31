/*****************************************************************************
 *                                                                           *
 * The breve Simulation Environment                                          *
 * Copyright (C) 2000, 2001, 2002, 2003 Jonathan Klein                       *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program; if not, write to the Free Software               *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA *
 *****************************************************************************/

/** 
 * English error titles and strings.
 */

char *gErrorNames[] = {
    "Unknown Error",
    "Parse Error",
    "Python Error",
    "Syntax Error",
    "Internal Error",
    "Unknown Symbol Error",
    "Unknown Function Error",
    "Unknown Object Error",
    "Redefinition Error",
    "Type Error",
    "Function Prototype Error",
    "No Controller Error",
    "Incompatible File Version",

    // EVALUATION ERRORS 

    "Unknown Controller Error",
    "Simulation Triggered Error",
    "Internal Error",
    "Type Error",
    "Array Error",
    "Conversion Error",
    "Freed Instance Error",
    "NULL Instance Error",
    "Math Error",
    "Simulation Error",
    "Bounds Error",
    "Unknown Object Error",
    "Unknown Method Error",
    "Unknown Keyword Error",
    "Missing Keyword Error",
    "File Not Found",
    NULL
};

char *gErrorMessages[] = {
    "An unknown error occurred (see the breve log for more information).",
    "A \"parse error\" is a very general error indicating that the parser encountered unexpected input.\n\nThe error can sometimes be hard to track down, as it does not always appear on the line specified by this error message--it is possible that the error occurs on a preceding line but is not discovered until the line specified.  Check for missing punctuation, misspellings and missing keywords in method calls.",
    "A \"Python language error\" indicates that an error occurred while executing the Python code for this simulation.  See the breve log for more detailed error information.",
    "A \"syntax error\" indicates that a specific syntax error was encountered while parsing the simulation.\n\nThe error description above should describe the problem and how it can be fixed.",
    "An \"internal error\" indicates that a bug was encountered in this program while parsing the simulation.\n\nPlease file a bug report which includes a copy of the simulation which triggered this bug.",
    "An \"unknown symbol error\" indicates that an unknown variable was referenced.\n\nCheck that the variable is properly declared and that the variable name is spelled correctly.",
    "An \"unknown function error\" indicates that a call was made to an unknown function.\n\nConfirm that the function exists and that the function name is spelled correctly.",
    "An \"unknown object error\" indicates that a reference was made to an unknown object type.\n\nConfirm that the right class files are imported and that the class name are spelled correctly.",
    "A \"redefinition error\" indicates that a symbol (such as a class name, variable or method name) has been defined more than once."
    "A \"type error\" indicates that the parser expected a different datatype while parsing an expression.",
    "A \"function prototype error\" indicates the wrong number of arguments were passed to the specified function."
	"An \"incompatible file version\" error indicates that the simulation could not continue because one of the files required by the simulation is an incompatible version for this simulation.\n\nSee the log window to determine which class file needs to be replaced.",

     // EVALUATION ERRORS 

    "A \"simulation triggered error\" indicates that this error was triggered manually from within the simulation using the \"die\" command.",
    "An \"internal error\" indicates that a bug was encountered in this program during the simulation.\n\nPlease file a bug report which includes a copy of the simulation which triggered this bug.",
    "A \"type error\" indicates that a certain expression expected one type of value, but was given another.\n\nThe error text should indicate that a certain type was expected, or, in some cases that a certain type is explicitly not allowed in the specified expression.  Not all operators can be used on all types--make sure that the type operator you are using is compatible with the type it is given.",
    "An \"array error\" indicates that a variable defined as an array is being used as a normal variable.\n\nThe only legal way to use an array variable is in conjunction with an array index to access or assign a specific member of the array.",
    "A \"conversion error\" indicates that an error occurred while attempting to convert one type to another.\n\nThis error indicates that a certain type was expected for an evaluation, that the value given did not match the desired type and that if could not be converted to the desired type.",
    "A \"freed instance error\" indicates that a freed instance is being used in the simulation, most likely with a method call.\n\nIf objects may be freed during the course of your simulation, they should be tested before they are passed method calls.",
    "A \"null instance error\" indicates that a method is being called with an uninitialized instance.\n\nAll objects must be instantiated using the \"new\" operator before they are passed messages.",
    "A \"math error\" indicates that an error occurred while evaluating a mathematical expression.\n\nThe error indicates that invalid values were used in the mathematical expression, such as a division or modulus by zero.  Make sure you check for these cases explicitly in your simulation when they are possible.",
    "A \"simulation error\" indicates that an error occurred while simulating the world.  Simulation errors are most commonly caused by numerical overflows during simulation and are not easy to track down.",
    "A \"bounds error\" indicates that the simulation attempted to access or assign an element of a list which is too large.  Make sure that you do not try to access or assign elements with indexes larger than the length of the list.",
    "An \"unknown object\" error indicates that the \"new\" operator was called with an object name that could not be found.\n\nMake sure that the object file is included and that the object name is spelled correctly.",
    "An \"unknown method error\" indicates that the method called in an object could not be found.\n\nCheck the definition of the target object type and confirm that the method is defined.",
    "An \"unknown keyword error\" indicates that one of the keywords passed to the method in question is unknown.\n\nCheck that the keywords passed to the method match the keywords defined in the method definition.",
    "A \"missing keyword error\" indicates that at least one keyword is missing from the method call in question.\n\nCheck the definition of the method and make sure that all the keywords are passed in.",
	"A \"file not found\" error indicates that the simulation could not continue because a necessary file was not found.",

    NULL
};
