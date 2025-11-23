#!/usr/bin/env python3
# generate_matrix.py
# Generate test matrices for matrix multiplication benchmarking

import numpy as np
import argparse
import struct

def generate_matrix(n, matrix_type='random', seed=None):
    """Generate a matrix of size n x n.
    
    Args:
        n: Matrix dimension
        matrix_type: Type of matrix ('random', 'identity', 'zeros', 'ones')
        seed: Random seed for reproducibility
    
    Returns:
        numpy array of shape (n, n)
    """
    if seed is not None:
        np.random.seed(seed)
    
    if matrix_type == 'random':
        return np.random.rand(n, n)
    elif matrix_type == 'identity':
        return np.eye(n)
    elif matrix_type == 'zeros':
        return np.zeros((n, n))
    elif matrix_type == 'ones':
        return np.ones((n, n))
    else:
        raise ValueError(f"Unknown matrix type: {matrix_type}")

def save_matrix_binary(matrix, filename):
    """Save matrix in binary format for C programs.
    
    Format: [n (int32)] [n*n doubles in row-major order]
    """
    n = matrix.shape[0]
    with open(filename, 'wb') as f:
        # Write matrix size
        f.write(struct.pack('i', n))
        # Write matrix data in row-major order (C order)
        f.write(matrix.tobytes(order='C'))
    print(f"Saved {n}x{n} matrix to {filename}")

def save_matrix_text(matrix, filename):
    """Save matrix in human-readable text format."""
    np.savetxt(filename, matrix, fmt='%.6f')
    print(f"Saved {matrix.shape[0]}x{matrix.shape[1]} matrix to {filename}")

def main():
    parser = argparse.ArgumentParser(
        description='Generate test matrices for matrix multiplication'
    )
    parser.add_argument('-n', '--size', type=int, default=1024,
                        help='Matrix dimension (default: 1024)')
    parser.add_argument('-t', '--type', choices=['random', 'identity', 'zeros', 'ones'],
                        default='random', help='Matrix type (default: random)')
    parser.add_argument('-o', '--output', default='matrix.bin',
                        help='Output filename (default: matrix.bin)')
    parser.add_argument('--text', action='store_true',
                        help='Save in text format instead of binary')
    parser.add_argument('--seed', type=int, default=None,
                        help='Random seed for reproducibility')
    
    args = parser.parse_args()
    
    print(f"Generating {args.size}x{args.size} {args.type} matrix...")
    matrix = generate_matrix(args.size, args.type, args.seed)
    
    if args.text:
        save_matrix_text(matrix, args.output)
    else:
        save_matrix_binary(matrix, args.output)
    
    print("Done!")

if __name__ == '__main__':
    main()
