# Vibescript-and-Vibebuilder
VibeScript: Custom Language Engineering with Interactive VibeBuilder Compilation

Project Abstract
VibeScript is a beginner-friendly programming language designed with culturally relatable, intuitive syntax to reduce cognitive load for new learners. It mirrors modern conversational styles, making code feel natural and accessible.

To bring VibeScript to life, we developed VibeBuilder, an interactive compiler that visually demonstrates each stage of compilation in real time—from lexical analysis to code execution. This tool transforms the traditional compiler from a “black box” into an engaging learning platform, helping users build a clear mental model of compiler internals.

VibeBuilder is built with:

ANTLR4 and Java for backend compiler logic,

React.js + D3.js for frontend visual interaction,

Monaco Editor as the code editor interface.

VibeScript compiles to JavaScript, enabling seamless execution within the browser. This system is tailored for educational use, empowering learners to explore programming and compiler concepts interactively and intuitively.

Project Architecture and Approach
Lexer and Parser: Implemented using Flex and Bison, based on a culturally relevant grammar tailored for beginners.

AST Construction: Managed via ast.h, with semantic analysis, code generation, and interpretation handled by modules like vibe.c, interpreter.c, and vibe.h.

Interpretation vs. Compilation: Supports both direct interpretation and transpilation to intermediate forms.

GUI: Planned to feature real-time AST visualization and code interaction, enhancing learner feedback and understanding. GUI development is ongoing.

Modularity: The architecture supports testing, debugging, and education by making compiler internals transparent.

Tasks Completed
Task	Status
Lexical analyzer implemented (Flex)	Completed
Parser with Bison and AST generation	Completed
Interpreter logic for basic execution	Completed
GUI architecture design and planning	Completed

Challenges and Roadblocks
Integrating Bison-generated parser with AST logic for complex nested expressions.

Ensuring consistent interpreter behavior across different development environments.

Absence of GUI slowed visual debugging; compensated by enhanced console outputs.

Improving parser error handling for malformed inputs is ongoing.

Created standardized build scripts and setup documentation to ease environment consistency.

Pending Tasks
Task	Status
AST graphical enhancements	In Progress
Improved parser error messaging	In Progress
Full language feature support in interpreter	In Progress
GUI development and usability improvements	In Progress

Project Deliverables
Fully functional lexer and parser capable of processing VibeScript syntax.

Interpreter module with runtime code evaluation.

Planned GUI with integrated code editor, output view, and interactive AST visualizer.

Standardized build system and comprehensive documentation.

Educational focus on making compiler concepts transparent and accessible.

Progress Overview
Completed: Lexer, parser, basic interpreter features, and test suite.

In Progress: Interpreter expansion, GUI development, error handling enhancements.

Pending: Final integration, GUI implementation, advanced interpreter features, and documentation finalization.

Testing and Validation Status
Test Type	Status	Notes
Lexer Tokenization Test	Pass	Correct token generation for keywords, identifiers, and symbols
Parser Syntax Validation	Pass	All test cases in test.vibe parsed without errors
AST Generation Test	Pass	AST structure matches expected outputs
Interpreter Functionality	Pass (Basic)	Basic arithmetic and assignment work; control flow in progress
GUI Code-Output Sync Test	Pending	GUI not yet developed
Error Handling Test	Partial Pass	Basic syntax errors handled; detailed error messages under development
Cross-platform Compatibility	Pass	Project builds and runs successfully on Windows tested environment

Codebase Information
https://github.com/NikitaaBhatt/VibeScript-and-VibeBuilder-

Contact and Support
For questions or contributions, please contact the project team through the project repository.

Thank you for exploring VibeScript and VibeBuilder — making programming and compiler education more intuitive and engaging
