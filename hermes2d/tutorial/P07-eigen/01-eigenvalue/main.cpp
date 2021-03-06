#define HERMES_REPORT_ALL
#include "hermes2d.h"
#include <stdio.h>

using Teuchos::RCP;
using Teuchos::rcp;
using Hermes::EigenSolver;

//  This example solves the eigenproblem for the Laplace operator in 
//  a square with zero boundary conditions. Python and Pysparse must
//  be installed. 
//
//  PDE: -Laplace u = lambda_k u,
//  where lambda_0, lambda_1, ... are the eigenvalues.
//
//  Domain: Square (0, pi)^2.
//
//  BC:  Homogeneous Dirichlet.
//
//  The following parameters can be changed:

const int NUMBER_OF_EIGENVALUES = 50;             // Desired number of eigenvalues.
const int P_INIT = 4;                             // Uniform polynomial degree of mesh elements.
const int INIT_REF_NUM = 3;                       // Number of initial mesh refinements.
const double TARGET_VALUE = 2.0;                  // PySparse parameter: Eigenvalues in the vicinity of 
                                                  // this number will be computed. 
const double TOL = 1e-10;                         // Pysparse parameter: Error tolerance.
const int MAX_ITER = 1000;                        // PySparse parameter: Maximum number of iterations.

// Boundary markers.
const int BDY_ALL = 1;

// Weak forms.
#include "forms.cpp"

int main(int argc, char* argv[])
{
  info("Desired number of eigenvalues: %d.", NUMBER_OF_EIGENVALUES);

  // Load the mesh.
  Mesh mesh;
  H2DReader mloader;
  mloader.load("domain.mesh", &mesh);

  // Perform initial mesh refinements (optional).
  for (int i = 0; i < INIT_REF_NUM; i++) mesh.refine_all_elements();

  // Enter boundary markers. 
  BCTypes bc_types;
  bc_types.add_bc_dirichlet(BDY_ALL);

  // Enter Dirichlet boundary values.
  BCValues bc_values;
  bc_values.add_zero(BDY_ALL);

  // Create an H1 space with default shapeset.
  H1Space space(&mesh, &bc_types, &bc_values, P_INIT);
  int ndof = Space::get_num_dofs(&space);
  info("ndof: %d.", ndof);

  // Initialize the weak formulation for the left hand side, i.e., H.
  WeakForm wf_left, wf_right;
  wf_left.add_matrix_form(callback(bilinear_form_left));
  wf_right.add_matrix_form(callback(bilinear_form_right));

  // Initialize matrices.
  RCP<SparseMatrix> matrix_left = rcp(new CSCMatrix());
  RCP<SparseMatrix> matrix_right = rcp(new CSCMatrix());

  // Assemble matrices.
  bool is_linear = true;
  DiscreteProblem dp_left(&wf_left, &space, is_linear);
  dp_left.assemble(matrix_left.get());
  DiscreteProblem dp_right(&wf_right, &space, is_linear);
  dp_right.assemble(matrix_right.get());

  EigenSolver es(matrix_left, matrix_right);
  info("Calling Pysparse...");
  es.solve(NUMBER_OF_EIGENVALUES, TARGET_VALUE, TOL, MAX_ITER);
  info("Pysparse finished.");
  es.print_eigenvalues();

  // Initializing solution vector, solution and ScalarView.
  double* coeff_vec;
  Solution sln;
  ScalarView view("Solution", new WinGeom(0, 0, 440, 350));

  // Reading solution vectors and visualizing.
  double* eigenval = new double[NUMBER_OF_EIGENVALUES];
  int neig = es.get_n_eigs();
  if (neig != NUMBER_OF_EIGENVALUES) error("Mismatched number of eigenvalues.");  
  for (int ieig = 0; ieig < neig; ieig++) {
    eigenval[ieig] = es.get_eigenvalue(ieig);
    int n;
    es.get_eigenvector(ieig, &coeff_vec, &n);
    // Convert coefficient vector into a Solution.
    Solution::vector_to_solution(coeff_vec, &space, &sln);

    // Visualize the solution.
    char title[100];
    sprintf(title, "Solution %d, val = %g", ieig, eigenval[ieig]);
    view.set_title(title);
    view.show(&sln);

    // Wait for keypress.
    View::wait(HERMES_WAIT_KEYPRESS);
  }

  return 0; 
};

