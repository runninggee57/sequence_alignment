#include <stdio.h>
#include <stdlib.h>

const int DEFAULT_SEQ_SIZE = 100;
const int INFINITY = 32767;
const char EOL = '\n';

struct gridCell {
    int row;
    int col;
    int val;
    struct gridCell *bestAdj;
};

// Prototype Declarations
int getIndexFromArray(char char_1, char *array);
int max2(int a, int b);
int max4(int a, int b, int c, int d);

int getIndexFromArray(char char_1, char *array) {
    int i;
    for (i = 0; i < sizeof(array)/sizeof(char); i++) {
        if (array[i] == char_1) {
            return i;
        }
    }
    return -1;
}

int max2(int a, int b) {
    if (a >= b) {
        return a;
    }
    else {
        return b;
    }
}

int max4(int a, int b, int c, int d) {
    if (max2(a, b) >= max2(c, d)) {
        return max2(a, b);
    }
    else {
        return max2(c, d);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 4) {
        printf("Invalid number of arguments\n");
        printf("Align sequence_filename sub_matrix_filename gap_init_pen gap_ext_pen");
        exit(EXIT_FAILURE);
    }

    int pen_d = atoi(argv[3]); // gap initialization penalty
    int pen_e = atoi(argv[4]); // gap extension penalty

    char *seq1 = malloc(DEFAULT_SEQ_SIZE * sizeof(char));
    int size_seq1 = 0;
    char *seq2 = malloc(DEFAULT_SEQ_SIZE * sizeof(char));
    int size_seq2 = 0;


    FILE *seqFile;
    if ((seqFile = fopen(argv[1], "r")) == NULL) {
        printf("Could not read sequence file: %s", argv[1]);
        exit(EXIT_FAILURE);
    }

    char in_char;
    while ((in_char = fgetc(seqFile)) != EOL) {
        size_seq1++;
        seq1[size_seq1 - 1] = in_char;
        if (size_seq1 % DEFAULT_SEQ_SIZE == 0) {
            seq1 = realloc(seq1, (size_seq1 + DEFAULT_SEQ_SIZE) * sizeof(char));
        }
    }
    seq1 = realloc(seq1, size_seq1 * sizeof(char));

    while ((in_char = fgetc(seqFile)) != EOL) {
        size_seq2++;
        seq2[size_seq2 - 1] = in_char;
        if (size_seq2 % DEFAULT_SEQ_SIZE == 0) {
            seq2 = realloc(seq2, (size_seq2 + DEFAULT_SEQ_SIZE) * sizeof(char));
        }
    }
    seq2 = realloc(seq2, size_seq2 * sizeof(char));

    fclose(seqFile);

    // Create Substitution Matrix
    FILE *subFile;
    if ((subFile = fopen(argv[2], "r")) == NULL) {
        printf("Could not read substitution matrix file: %s\n", argv[2]);
        exit(EXIT_FAILURE);
    }

    char sub_row_chars[20];
    char sub_col_chars[20];
    int sub_mat[20][20];

    int i;
    for (i = 0; i <= 20; i++) { //rows of sub matrix
        int j;
        for (j = 0; j <= 20; j++) { // cols of sub matrix
            if (i == 0) { // reading letters from top row
                while ((in_char = fgetc(subFile)) != EOL) {
                    char int_str[4]; // sub matrix shouldn't have #s longer than -99
                    if (in_char == ' ') {
                        continue;
                    }
                    else {
                        if (i == 0) { // reading single letters from top row
                            sub_col_chars[j] = in_char;
                        }
                        else if (j == 0) { // reading char identifying a row
                            sub_row_chars[i - 1] = in_char;
                        }
                        else {
                            int index = 0;
                            while (in_char != ' ') {
                                int_str[index] = in_char;
                                in_char = fgetc(subFile);
                                index++;
                            }
                            int_str[index] = NULL; // Null terminate the string
                            sub_mat[i - 1][j - 1] = atoi(int_str);
                        }
                    }
                }
            }
        }
    }

    fclose(subFile);

    struct gridCell M[size_seq1][size_seq2];
    struct gridCell Ix[size_seq1][size_seq2];
    struct gridCell Iy[size_seq1][size_seq2];

    // Initialize matrices
    for (i = 0; i < size_seq1; i++){
        (M[i][0]).val = 0;
        (M[i][0]).row = i;
        (M[i][0]).col = 0;
        (Ix[i][0]).val = -1 * INFINITY;
        (Ix[i][0]).row = i;
        (Ix[i][0]).col = 0;
        (Iy[i][0]).val = -1 * INFINITY;
        (Iy[i][0]).row = i;
        (Iy[i][0]).col = 0;
    }
    int j;
    for (j = 0; j < size_seq2; j++) {
        (M[0][j]).val = 0;
        (M[0][j]).row = 0;
        (M[0][j]).col = j;
        (Ix[0][j]).val = -1 * INFINITY;
        (Ix[0][j]).row = 0;
        (Ix[0][j]).col = j;
        (Iy[0][j]).val = -1 * INFINITY;
        (Iy[0][j]).row = 0;
        (Iy[0][j]).col = j;
    }

    // Populate matrices
    struct gridCell max_gridcell = {0, NULL};
    for (i = 1; i < size_seq1; i++) { //rows
        int j;
        for (j = 1; j < size_seq2; j++) { //cols
            // M matrix
            (M[i][j]).row = i;
            (M[i][j]).col = j;
            int sub_value = sub_mat[getIndexFromArray(seq1[i], sub_row_chars)][getIndexFromArray(seq2[i], sub_col_chars)];
            int m_score = (M[i - 1][j - 1]).val + sub_value;
            int ix_score = (Ix[i - 1][j - 1]).val + sub_value;
            int iy_score = (Iy[i - 1][j - 1]).val + sub_value;
            int max_val = max4(m_score, ix_score, iy_score, 0);
            if (max_val == ix_score) {
                (M[i][j]).val = max_val;
                (M[i][j]).bestAdj = &(Ix[i - 1][j - 1]);
                if (max_val > max_gridcell.val) {
                    max_gridcell = M[i][j];
                }
            }
            else if (max_val == m_score) {
                (M[i][j]).val = max_val;
                (M[i][j]).bestAdj = &(M[i - 1][j - 1]);
                if (max_val > max_gridcell.val) {
                    max_gridcell = M[i][j];
                }
            }
            else if (max_val == iy_score) {
                (M[i][j]).val = max_val;
                (M[i][j]).bestAdj = &(Iy[i - 1][j - 1]);
                if (max_val > max_gridcell.val) {
                    max_gridcell = M[i][j];
                }
            }
            else {
                (M[i][j]).val = 0;
                (M[i][j]).bestAdj = NULL;
            }

            // Ix matrix
            (Ix[i][j]).row = i;
            (Ix[i][j]).col = j;
            m_score = (M[i - 1][j]).val - pen_d;
            ix_score = (Ix[i-1][j]).val - pen_e;
            max_val = max2(m_score, ix_score);
            if (max_val == m_score) {
                (Ix[i][j]).val = max_val;
                (Ix[i][j]).bestAdj = &(M[i - 1][j]);
                if (max_val > max_gridcell.val) {
                    max_gridcell = Ix[i][j];
                }
            }
            else {
                (Ix[i][j]).val = max_val;
                (Ix[i][j]).bestAdj = &(Ix[i - 1][j]);
                if (max_val > max_gridcell.val) {
                    max_gridcell = Ix[i][j];
                }
            }

            // Iy matrix
            (Iy[i][j]).row = i;
            (Iy[i][j]).col = j;
            m_score = (M[i][j - 1]).val - pen_d;
            iy_score = (Iy[i][j - 1]).val - pen_e;
            max_val = max2(m_score, iy_score);
            if (max_val == m_score) {
                (Iy[i][j]).val = max_val;
                (Iy[i][j]).bestAdj = &(M[i][j - 1]);
                if (max_val > max_gridcell.val) {
                    max_gridcell = Iy[i][j];
                }
            }
            else {
                (Iy[i][j]).val = max_val;
                (Iy[i][j]).bestAdj = &(Iy[i][j - 1]);
                if (max_val > max_gridcell.val) {
                    max_gridcell = Ix[i][j];
                }
            }
        }
    }

    // Traceback, storing alignment
    int done = 0;
    char *new_seq1 = malloc(size_seq1 * sizeof(char));
    int size_new_seq1 = 0;
    char *new_seq2 = malloc(size_seq2 * sizeof(char));
    int size_new_seq2 = 0;
    struct gridCell cur_cell = max_gridcell;
    int cur_seq1_index = 0;
    int cur_seq2_index = 0;
    int last_row = -1;
    int last_col = -1;
    while (!done) {
        if (last_row == cur_cell.row) {
            new_seq1[cur_seq1_index] = '-';
        }
        else {
            new_seq1[cur_seq1_index] = sub_row_chars[cur_cell.row];
        }

        if (last_col == cur_cell.col) {
            new_seq1[cur_seq2_index] = '-';
        }
        else {
            new_seq2[cur_seq2_index] = sub_row_chars[cur_cell.col];
        }

        size_new_seq1++;
        size_new_seq2++;
        last_row = cur_cell.row;
        last_col = cur_cell.col;
        cur_cell = *(cur_cell.bestAdj);
        if (cur_cell.val == 0) {
            done = 1;
        }
    }

    // Resize and make room for null termination
    new_seq1 = realloc(new_seq1, (size_new_seq1 + 1) * sizeof(char));
    new_seq1[size_new_seq2] = NULL;
    new_seq2 = realloc(new_seq2, (size_new_seq2 + 1) * sizeof(char));
    new_seq2[size_new_seq2] = NULL;

    // Reverse sequences
    for (i = 0; i < size_new_seq1 / 2; i++) {
        char temp = new_seq1[i];
        new_seq1[i] = new_seq1[size_new_seq1 - 1 - i];
        new_seq1[size_new_seq1 - 1 - i] = temp;
    }

    // Print results

    printf("Sequence 1: %s\n\n", new_seq1);
    printf("Sequence 2: %s\n\n", new_seq2);

    free(seq1);
    free(seq2);
    free(new_seq1);
    free(new_seq2);

    return 0;
}
