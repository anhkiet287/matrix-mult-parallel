#!/bin/bash
# Submit MPI + hybrid sweeps to a Slurm cluster (HPCC/Gateway ready).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

if ! command -v sbatch >/dev/null 2>&1; then
    cat <<EOF
Slurm (sbatch) not detected on this system.
To run manually, SSH into a Slurm-enabled cluster and execute:

  cd $PROJECT_ROOT
  RESULTS_DIR=$PROJECT_ROOT/results MACHINE_ID=\$(hostname) RESULTS_NOTE="manual cluster run" \\
    MPI_PROC_LIST="2,4,8" HYBRID_GRID="2x8,4x6" bash scripts/run_tests.sh
EOF
    exit 1
fi

JOB_SCRIPT="$(mktemp /tmp/matmul_slurm_job.XXXXXX)"
cat >"$JOB_SCRIPT" <<EOF
#!/bin/bash
#SBATCH --job-name=matmul-sweeps
#SBATCH --output=$PROJECT_ROOT/logs/slurm_%j.out
#SBATCH --error=$PROJECT_ROOT/logs/slurm_%j.err
#SBATCH --time=01:00:00
#SBATCH --nodes=1
#SBATCH --ntasks=8
#SBATCH --cpus-per-task=4

module purge >/dev/null 2>&1 || true
cd "$PROJECT_ROOT"

export RESULTS_DIR="\${RESULTS_DIR:-$PROJECT_ROOT/results}"
export MACHINE_ID="\${SLURM_NODELIST:-slurm}"
export RESULTS_NOTE="slurm job \$SLURM_JOB_ID"
export MPI_PROC_LIST="\${MPI_PROC_LIST:-2,4,8}"
export HYBRID_GRID="\${HYBRID_GRID:-2x8,4x4}"

mkdir -p "\$RESULTS_DIR" "$PROJECT_ROOT/logs"

echo "Running MPI sweeps with MPI_PROC_LIST=\$MPI_PROC_LIST"
srun bash scripts/run_tests_mpi.sh

echo "Running hybrid sweeps with HYBRID_GRID=\$HYBRID_GRID"
srun bash scripts/run_tests_hybrid.sh
EOF

chmod +x "$JOB_SCRIPT"
JOB_ID=$(sbatch "$JOB_SCRIPT")
echo "Submitted Slurm job: $JOB_ID"
echo "Temporary job script: $JOB_SCRIPT"
