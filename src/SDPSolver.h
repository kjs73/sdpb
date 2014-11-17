#ifndef SDP_BOOTSTRAP_SDPSOLVER_H_
#define SDP_BOOTSTRAP_SDPSOLVER_H_

#include <iostream>
#include <ostream>
#include <vector>
#include "boost/filesystem.hpp"
#include "types.h"
#include "Vector.h"
#include "Matrix.h"
#include "BlockDiagonalMatrix.h"
#include "SDP.h"

using std::vector;
using std::ostream;
using std::endl;
using boost::filesystem::path;

class SDPSolverParameters {
public:
  int maxIterations;
  int maxRuntime;
  int checkpointInterval;
  bool noFinalCheckpoint;
  bool findPrimalFeasible;
  bool findDualFeasible;
  bool detectPrimalFeasibleJump;
  bool detectDualFeasibleJump;
  int precision;
  int maxThreads;
  Real dualityGapThreshold;
  Real primalErrorThreshold;
  Real dualErrorThreshold;
  Real initialMatrixScalePrimal;
  Real initialMatrixScaleDual;
  Real feasibleCenteringParameter;
  Real infeasibleCenteringParameter;
  Real stepLengthReduction;
  Real choleskyStabilizeThreshold;
  Real maxComplementarity;

  void resetPrecision() {
    setPrecision(dualityGapThreshold,          precision);
    setPrecision(primalErrorThreshold,         precision);
    setPrecision(dualErrorThreshold,           precision);
    setPrecision(initialMatrixScalePrimal,     precision);
    setPrecision(initialMatrixScaleDual,       precision);
    setPrecision(feasibleCenteringParameter,   precision);
    setPrecision(infeasibleCenteringParameter, precision);
    setPrecision(stepLengthReduction,          precision);
    setPrecision(choleskyStabilizeThreshold,   precision);
    setPrecision(maxComplementarity,           precision);
  }

  friend ostream& operator<<(ostream& os, const SDPSolverParameters& p);
};

enum SDPSolverTerminateReason {
  PrimalDualOptimal,
  PrimalFeasible,
  DualFeasible,
  PrimalFeasibleJumpDetected,
  DualFeasibleJumpDetected,
  MaxComplementarityExceeded,
  MaxIterationsExceeded,
  MaxRuntimeExceeded,
};

ostream &operator<<(ostream& os, const SDPSolverTerminateReason& r);

class SDPSolverStatus {
public:
  Real primalObjective;
  Real dualObjective;
  Real primalError;
  Real dualError;

  Real dualityGap() const {
    return abs(primalObjective - dualObjective) /
      max(Real(abs(primalObjective) + abs(dualObjective)), Real(1));
  }

  friend ostream& operator<<(ostream& os, const SDPSolverStatus& s);
};

class SDPSolver {
public:
  SDP sdp;
  SDPSolverStatus status;

  // current point
  Vector x;
  BlockDiagonalMatrix X;
  Vector y;
  BlockDiagonalMatrix Y;

  // search direction
  Vector dx;
  BlockDiagonalMatrix dX;
  Vector dy;
  BlockDiagonalMatrix dY;

  // discrepancies in dual and primal equality constraints
  Vector dualResidues;
  BlockDiagonalMatrix PrimalResidues;

  // intermediate computations
  BlockDiagonalMatrix XCholesky;
  BlockDiagonalMatrix YCholesky;
  BlockDiagonalMatrix Z;
  BlockDiagonalMatrix R;
  BlockDiagonalMatrix BilinearPairingsXInv;
  BlockDiagonalMatrix BilinearPairingsY;
  BlockDiagonalMatrix SchurBlocks;
  BlockDiagonalMatrix SchurBlocksCholesky;
  Matrix SchurUpdateLowRank;

  vector<vector<int> > schurStabilizeIndices;
  vector<vector<Real> > schurStabilizeLambdas;
  vector<int> stabilizeBlockIndices;
  vector<int> stabilizeBlockUpdateRow;
  vector<int> stabilizeBlockUpdateColumn;
  vector<Matrix> stabilizeBlocks;

  Matrix Q;
  vector<Integer> Qpivots;
  Vector basicKernelCoords;

  // additional workspace variables
  BlockDiagonalMatrix StepMatrixWorkspace;
  vector<Matrix> bilinearPairingsWorkspace;
  vector<Vector> eigenvaluesWorkspace;
  vector<Vector> QRWorkspace;

  SDPSolver(const SDP &sdp);
  void initialize(const SDPSolverParameters &parameters);
  SDPSolverTerminateReason run(const SDPSolverParameters &parameters, const path checkpointFile);
  void initializeSchurComplementSolver(const BlockDiagonalMatrix &BilinearPairingsXInv,
                                       const BlockDiagonalMatrix &BilinearPairingsY,
                                       const Real &choleskyStabilizeThreshold);
  void solveSchurComplementEquation(Vector &dx, Vector &dz);
  void computeSearchDirection(const Real &beta, const Real &mu, const bool correctorPhase);
  Vector freeVariableSolution();
  void saveCheckpoint(const path &checkpointFile);
  void loadCheckpoint(const path &checkpointFile);
  void saveSolution(const SDPSolverTerminateReason, const path &outFile);
};

void printSolverHeader();
void printSolverInfo(int iteration,
                     Real mu,
                     SDPSolverStatus status,
                     Real primalStepLength,
                     Real dualStepLength,
                     Real betaCorrector,
                     int dualObjectiveSize,
                     int Qrows);

#endif  // SDP_BOOTSTRAP_SDPSOLVER_H_
