#include <gtk/gtk.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

/*===================================================================
                        FONCTIONS DE CRYPTOGRAPHIE
=======================================================================*/

/*===================================================================
                        PLAYFAIR (I/J separated, W->V)
=======================================================================*/

void preparer_texte_playfair(char *texte) {
    int i, j = 0;
    for (i = 0; texte[i] != '\0'; i++) {
        if (isalpha(texte[i])) {
            char c = toupper(texte[i]);
            if (c == 'W') c = 'V';  // W is replaced with V
            texte[j++] = c;
        }
    }
    texte[j] = '\0';
}

void creer_matrice_playfair(const char *cle, char matrice[5][5]) {
    int used[26] = {0};
    int row = 0, col = 0;
    int i;
    
    // Process the key
    for (i = 0; cle[i] != '\0'; i++) {
        if (isalpha(cle[i])) {
            char c = toupper(cle[i]);
            if (c == 'W') c = 'V';  // W -> V
            
            int index = c - 'A';
            if (!used[index]) {
                used[index] = 1;
                matrice[row][col] = c;
                col++;
                if (col == 5) {
                    col = 0;
                    row++;
                }
            }
        }
    }
    
    // Fill with remaining alphabet (excluding W only, I and J are separate)
    for (char c = 'A'; c <= 'Z'; c++) {
        if (c == 'W') continue;  // Only W is excluded
        int index = c - 'A';
        if (!used[index]) {
            matrice[row][col] = c;
            col++;
            if (col == 5) {
                col = 0;
                row++;
            }
        }
    }
}

void trouver_position(char matrice[5][5], char c, int *row, int *col) {
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (matrice[i][j] == c) {
                *row = i;
                *col = j;
                return;
            }
        }
    }
}

void chiffrer_bigramme_playfair(const char *bigramme, char matrice[5][5], char *resultat) {
    char c1 = toupper(bigramme[0]);
    char c2 = toupper(bigramme[1]);
    
    // W -> V conversion
    if (c1 == 'W') c1 = 'V';
    if (c2 == 'W') c2 = 'V';
    
    // Handle duplicate letters
    if (c1 == c2) {
        c2 = 'X';
    }
    
    int r1, c1_col, r2, c2_col;
    trouver_position(matrice, c1, &r1, &c1_col);
    trouver_position(matrice, c2, &r2, &c2_col);
    
    if (r1 == r2) {
        // Same row: shift right
        resultat[0] = matrice[r1][(c1_col + 1) % 5];
        resultat[1] = matrice[r2][(c2_col + 1) % 5];
    } else if (c1_col == c2_col) {
        // Same column: shift down
        resultat[0] = matrice[(r1 + 1) % 5][c1_col];
        resultat[1] = matrice[(r2 + 1) % 5][c2_col];
    } else {
        // Rectangle: swap columns
        resultat[0] = matrice[r1][c2_col];
        resultat[1] = matrice[r2][c1_col];
    }
}

void dechiffrer_bigramme_playfair(const char *bigramme_chiffre, char matrice[5][5], char *bigramme_clair) {
    char c1 = toupper(bigramme_chiffre[0]);
    char c2 = toupper(bigramme_chiffre[1]);
    
    if (c1 == 'W') c1 = 'V';
    if (c2 == 'W') c2 = 'V';
    
    int r1, c1_col, r2, c2_col;
    trouver_position(matrice, c1, &r1, &c1_col);
    trouver_position(matrice, c2, &r2, &c2_col);
    
    if (r1 == r2) {
        // Same row: shift left
        bigramme_clair[0] = matrice[r1][(c1_col + 4) % 5];
        bigramme_clair[1] = matrice[r2][(c2_col + 4) % 5];
    } else if (c1_col == c2_col) {
        // Same column: shift up
        bigramme_clair[0] = matrice[(r1 + 4) % 5][c1_col];
        bigramme_clair[1] = matrice[(r2 + 4) % 5][c2_col];
    } else {
        // Rectangle: swap columns
        bigramme_clair[0] = matrice[r1][c2_col];
        bigramme_clair[1] = matrice[r2][c1_col];
    }
}

char *chiffrer_playfair(const char *texte, char matrice[5][5]) {
    char *texte_prepare = strdup(texte);
    preparer_texte_playfair(texte_prepare);
    
    int len = strlen(texte_prepare);
    if (len % 2 == 1) {
        texte_prepare = realloc(texte_prepare, len + 2);
        texte_prepare[len] = 'X';
        texte_prepare[len + 1] = '\0';
        len++;
    }
    
    char *resultat = malloc(len + 1);
    int pos = 0;
    
    for (int i = 0; i < len; i += 2) {
        char bigramme[3] = {texte_prepare[i], texte_prepare[i+1], '\0'};
        char chiffre[2];
        chiffrer_bigramme_playfair(bigramme, matrice, chiffre);
        resultat[pos++] = chiffre[0];
        resultat[pos++] = chiffre[1];
    }
    resultat[pos] = '\0';
    
    free(texte_prepare);
    return resultat;
}

char *dechiffrer_playfair(const char *texte_chiffre, char matrice[5][5]) {
    int len = strlen(texte_chiffre);
    char *resultat = malloc(len + 1);
    int pos = 0;
    
    for (int i = 0; i < len; i += 2) {
        if (i + 1 < len) {
            char bigramme[3] = {texte_chiffre[i], texte_chiffre[i+1], '\0'};
            char clair[2];
            dechiffrer_bigramme_playfair(bigramme, matrice, clair);
            resultat[pos++] = clair[0];
            resultat[pos++] = clair[1];
        }
    }
    resultat[pos] = '\0';
    
    return resultat;
}


/*===================================================================
                    RECONSTRUCTION DE MATRICE PLAYFAIR - FIXED
=======================================================================*/

typedef struct {
    char clair[3];
    char chiffre[3];
    int type;
} PaireAnalysee;

int tester_matrice_avec_paires(char matrice[5][5], PaireAnalysee *paires, int n_paires) {
    int score = 0;
    
    for (int i = 0; i < n_paires; i++) {
        char resultat[3];
        chiffrer_bigramme_playfair(paires[i].clair, matrice, resultat);
        
        if (resultat[0] == paires[i].chiffre[0] && resultat[1] == paires[i].chiffre[1]) {
            score++;
        }
    }
    
    return score;
}

void generer_matrices_candidates(PaireAnalysee *paires, int n_paires, 
                                 char matrices_possibles[][5][5], int *n_matrices, int max_matrices) {
    *n_matrices = 0;
    
    // Strategy 1: Standard alphabetical matrix (excluding W only)
    if (*n_matrices < max_matrices) {
        char matrice[5][5];
        int pos = 0;
        for (char c = 'A'; c <= 'Z' && pos < 25; c++) {
            if (c == 'W') continue;  // Only exclude W
            matrice[pos / 5][pos % 5] = c;
            pos++;
        }
        memcpy(matrices_possibles[*n_matrices], matrice, sizeof(char) * 25);
        (*n_matrices)++;
    }
    
    // Strategy 2-4: Variations with offsets
    for (int offset = 1; offset <= 3 && *n_matrices < max_matrices; offset++) {
        char matrice[5][5];
        int pos = 0;
        for (int i = 0; i < 26 && pos < 25; i++) {
            char c = 'A' + ((i + offset * 5) % 26);
            if (c == 'W') continue;  // Only exclude W
            matrice[pos / 5][pos % 5] = c;
            pos++;
        }
        memcpy(matrices_possibles[*n_matrices], matrice, sizeof(char) * 25);
        (*n_matrices)++;
    }
    
    // Strategy 5: Based on letters from known pairs
    if (*n_matrices < max_matrices) {
        char matrice[5][5];
        int pos = 0;
        int used[26] = {0};
        
        // Start with letters from pairs
        for (int p = 0; p < n_paires && pos < 25; p++) {
            for (int i = 0; i < 2 && pos < 25; i++) {
                char c = toupper(paires[p].clair[i]);
                if (c == 'W') c = 'V';
                int idx = c - 'A';
                if (!used[idx]) {
                    matrice[pos / 5][pos % 5] = c;
                    used[idx] = 1;
                    pos++;
                }
            }
        }
        
        // Fill with rest of alphabet (excluding W)
        for (char c = 'A'; c <= 'Z' && pos < 25; c++) {
            if (c == 'W') continue;
            int idx = c - 'A';
            if (!used[idx]) {
                matrice[pos / 5][pos % 5] = c;
                pos++;
            }
        }
        
        memcpy(matrices_possibles[*n_matrices], matrice, sizeof(char) * 25);
        (*n_matrices)++;
    }
    
    // Fill rest with random permutations
    while (*n_matrices < max_matrices) {
        char matrice[5][5];
        char lettres[25];
        int n_lettres = 0;
        
        // Collect all valid letters (A-Z except W)
        for (char c = 'A'; c <= 'Z'; c++) {
            if (c == 'W') continue;
            lettres[n_lettres++] = c;
        }
        
        // Shuffle
        for (int i = n_lettres - 1; i > 0; i--) {
            int j = rand() % (i + 1);
            char temp = lettres[i];
            lettres[i] = lettres[j];
            lettres[j] = temp;
        }
        
        // Fill matrix
        for (int i = 0; i < 25; i++) {
            matrice[i / 5][i % 5] = lettres[i];
        }
        
        memcpy(matrices_possibles[*n_matrices], matrice, sizeof(char) * 25);
        (*n_matrices)++;
    }
}

void optimiser_matrice_avancee(char matrice[5][5], PaireAnalysee *paires, int n_paires, int iterations) {
    char meilleure[5][5];
    int meilleur_score = tester_matrice_avec_paires(matrice, paires, n_paires);
    
    memcpy(meilleure, matrice, sizeof(char) * 25);
    
    // Multiple random restarts approach
    int num_restarts = 15;  // Try 15 different starting points
    
    for (int restart = 0; restart < num_restarts; restart++) {
        char courante[5][5];
        
        if (restart == 0) {
            // First attempt: use provided matrix
            memcpy(courante, matrice, sizeof(char) * 25);
        } else {
            // Generate completely random matrix
            char lettres[25];
            int n_lettres = 0;
            for (char c = 'A'; c <= 'Z'; c++) {
                if (c == 'W') continue;
                lettres[n_lettres++] = c;
            }
            // Shuffle
            for (int i = n_lettres - 1; i > 0; i--) {
                int j = rand() % (i + 1);
                char temp = lettres[i];
                lettres[i] = lettres[j];
                lettres[j] = temp;
            }
            // Fill matrix
            for (int i = 0; i < 25; i++) {
                courante[i / 5][i % 5] = lettres[i];
            }
        }
        
        int score_courant = tester_matrice_avec_paires(courante, paires, n_paires);
        int iterations_per_restart = iterations / num_restarts;
        
        // Hill climbing with simulated annealing
        double temperature = 50.0;
        double cooling = 0.999;
        
        for (int iter = 0; iter < iterations_per_restart; iter++) {
            char test[5][5];
            memcpy(test, courante, sizeof(char) * 25);
            
            // Try different mutation strategies
            int strategy = rand() % 10;
            
            if (strategy < 6) {
                // 60%: Swap two random positions
                int r1 = rand() % 5, c1 = rand() % 5;
                int r2 = rand() % 5, c2 = rand() % 5;
                char temp = test[r1][c1];
                test[r1][c1] = test[r2][c2];
                test[r2][c2] = temp;
            } else if (strategy < 8) {
                // 20%: Swap two rows
                int r1 = rand() % 5;
                int r2 = rand() % 5;
                for (int c = 0; c < 5; c++) {
                    char temp = test[r1][c];
                    test[r1][c] = test[r2][c];
                    test[r2][c] = temp;
                }
            } else {
                // 20%: Swap two columns
                int c1 = rand() % 5;
                int c2 = rand() % 5;
                for (int r = 0; r < 5; r++) {
                    char temp = test[r][c1];
                    test[r][c1] = test[r][c2];
                    test[r][c2] = temp;
                }
            }
            
            int nouveau_score = tester_matrice_avec_paires(test, paires, n_paires);
            
            // Accept if better, or with probability based on temperature
            int accepter = 0;
            if (nouveau_score > score_courant) {
                accepter = 1;
            } else if (nouveau_score == score_courant) {
                accepter = (rand() % 100 < 30);  // 30% for equal
            } else {
                double delta = nouveau_score - score_courant;
                double prob = exp(delta / temperature);
                accepter = ((double)rand() / RAND_MAX < prob);
            }
            
            if (accepter) {
                score_courant = nouveau_score;
                memcpy(courante, test, sizeof(char) * 25);
                
                // Update global best
                if (nouveau_score > meilleur_score) {
                    meilleur_score = nouveau_score;
                    memcpy(meilleure, test, sizeof(char) * 25);
                    
                    // Perfect score? Stop immediately
                    if (meilleur_score == n_paires) {
                        memcpy(matrice, meilleure, sizeof(char) * 25);
                        return;
                    }
                }
            }
            
            temperature *= cooling;
        }
    }
    
    memcpy(matrice, meilleure, sizeof(char) * 25);
}

/*===================================================================
                        CESAR - VERSION ASCII COMPLETE
=======================================================================*/

char *chiffrer_cesar(const char *clair, int k)
{
    int n = strlen(clair);
    char *chiffre = malloc(n + 1);
    if (!chiffre) return NULL;

    k = k % 26;
    if (k < 0) k += 26;

    for (int i = 0; i < n; i++) {
        char c = clair[i];

        if (c >= 'A' && c <= 'Z') {
            chiffre[i] = (char)((c - 'A' + k) % 26 + 'A');
        }
        else if (c >= 'a' && c <= 'z') {
            chiffre[i] = (char)((c - 'a' + k) % 26 + 'a');
        }
        else {
            chiffre[i] = c;
        }
    }

    chiffre[n] = '\0';
    return chiffre;
}

char *decrypter_cesar(const char *chiffre, int k) {
    return chiffrer_cesar(chiffre, -k);
}

void calculer_frequence(const char *texte, double *frequences) {
    int total = 0;
    for (int i = 0; i < 26; i++)
        frequences[i] = 0.0;
    
    for (int i = 0; texte[i] != '\0'; i++) {
        if (isalpha(texte[i])) {
            frequences[toupper(texte[i]) - 'A']++;
            total++;
        }
    }
    
    if (total == 0) return;
    for (int i = 0; i < 26; i++)
        frequences[i] /= total;
}

int lettre_plus_frequente(const double *frequences) {
    int max = 0;
    for (int i = 1; i < 26; i++) {
        if (frequences[i] > frequences[max])
            max = i;
    }
    return max;
}

int trouver_cle_cesar(int lettre_freq) {
    int E = 'E' - 'A';
    return (lettre_freq - E + 26) % 26;
}

/*===================================================================
                        TRANSPOSITION
=======================================================================*/

char *chiffrer_transposition(const char *clair, int k) {
    int len = strlen(clair);
    char *chiffre = malloc(len + 1);
    int index = 0;
    if (!chiffre || k <= 0) return NULL;
    
    for (int col = 0; col < k; col++) {
        for (int i = col; i < len; i += k) {
            chiffre[index++] = clair[i];
        }
    }
    chiffre[len] = '\0';
    return chiffre;
}

char *dechiffrer_transposition(const char *chiffre, int k) {
    int len = strlen(chiffre);
    char *clair = malloc(len + 1);
    int rows = (len + k - 1) / k;
    int index = 0;
    if (!clair || k <= 0) return NULL;
    
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < k; col++) {
            int pos = col * rows + row;
            if (pos < len) {
                clair[index++] = chiffre[pos];
            }
        }
    }
    clair[len] = '\0';
    return clair;
}

void attaquer_transposition(const char *cryptogramme) {
    int len = strlen(cryptogramme);
    for (int k = 2; k <= len / 2; k++) {
        char *clair = dechiffrer_transposition(cryptogramme, k);
        if (clair) {
            printf("k = %d : %s\n", k, clair);
            free(clair);
        }
    }
}

/*===================================================================
                        STRUCTURE DE L'APPLICATION
=======================================================================*/

typedef struct {
    GtkWidget *window;
    GtkWidget *notebook;
    
    GtkWidget *cesar_input;
    GtkWidget *cesar_output;
    GtkWidget *cesar_key_spin;
    GtkWidget *cesar_freq_text;
    
    GtkWidget *trans_input;
    GtkWidget *trans_output;
    GtkWidget *trans_key_spin;
    GtkWidget *trans_attack_text;
    
    GtkWidget *playfair_input;
    GtkWidget *playfair_output;
    GtkWidget *playfair_key_entry;
    GtkWidget *playfair_matrix_text;
    
    GtkWidget *gen_key_entry;
    GtkWidget *gen_n_spin;
    GtkWidget *gen_flag_entry;
    GtkWidget *gen_output;
    
    GtkWidget *solver_paires_text;
    GtkWidget *solver_cryptogramme_entry;
    GtkWidget *solver_output;
} AppWidgets;

/*===================================================================
                        CALLBACKS - CESAR
=======================================================================*/

void on_cesar_chiffrer_clicked(GtkWidget *widget, gpointer data) {
    AppWidgets *app = (AppWidgets *)data;
    
    GtkTextBuffer *input_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->cesar_input));
    GtkTextBuffer *output_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->cesar_output));
    
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(input_buffer, &start, &end);
    char *texte = gtk_text_buffer_get_text(input_buffer, &start, &end, FALSE);
    
    int k = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(app->cesar_key_spin));
    
    if (texte && strlen(texte) > 0) {
        char *resultat = chiffrer_cesar(texte, k);
        if (resultat) {
            gtk_text_buffer_set_text(output_buffer, resultat, -1);
            free(resultat);
        }
    }
    
    g_free(texte);
}

void on_cesar_dechiffrer_clicked(GtkWidget *widget, gpointer data) {
    AppWidgets *app = (AppWidgets *)data;
    
    GtkTextBuffer *input_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->cesar_input));
    GtkTextBuffer *output_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->cesar_output));
    
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(input_buffer, &start, &end);
    char *texte = gtk_text_buffer_get_text(input_buffer, &start, &end, FALSE);
    
    int k = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(app->cesar_key_spin));
    
    if (texte && strlen(texte) > 0) {
        char *resultat = decrypter_cesar(texte, k);
        if (resultat) {
            gtk_text_buffer_set_text(output_buffer, resultat, -1);
            free(resultat);
        }
    }
    
    g_free(texte);
}

void on_cesar_analyser_clicked(GtkWidget *widget, gpointer data) {
    AppWidgets *app = (AppWidgets *)data;
    
    GtkTextBuffer *input_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->cesar_input));
    GtkTextBuffer *freq_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->cesar_freq_text));
    
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(input_buffer, &start, &end);
    char *texte = gtk_text_buffer_get_text(input_buffer, &start, &end, FALSE);
    
    if (!texte || strlen(texte) == 0) {
        gtk_text_buffer_set_text(freq_buffer, "Aucun texte a analyser!", -1);
        g_free(texte);
        return;
    }
    
    double frequences[26];
    calculer_frequence(texte, frequences);
    
    int lettre_freq = lettre_plus_frequente(frequences);
    int cle_estimee = trouver_cle_cesar(lettre_freq);
    
    char *resultat_dechiffre = decrypter_cesar(texte, cle_estimee);
    
    char buffer[4096];
    snprintf(buffer, sizeof(buffer), 
             "=== ANALYSE DE FREQUENCE ===\n\n"
             "Frequences des lettres:\n");
    
    for (int i = 0; i < 26; i++) {
        char temp[100];
        snprintf(temp, sizeof(temp), "%c: %5.2f%% ", 'A' + i, frequences[i] * 100);
        strcat(buffer, temp);
        if ((i + 1) % 5 == 0) strcat(buffer, "\n");
    }
    
    char conclusion[1024];
    snprintf(conclusion, sizeof(conclusion),
             "\n\n=== RESULTAT ===\n"
             "Lettre la plus frequente: %c (%.2f%%)\n"
             "Cle estimee: %d\n\n"
             "Dechiffrement avec cle %d:\n%s",
             'A' + lettre_freq, frequences[lettre_freq] * 100,
             cle_estimee, cle_estimee, resultat_dechiffre);
    strcat(buffer, conclusion);
    
    gtk_text_buffer_set_text(freq_buffer, buffer, -1);
    
    free(resultat_dechiffre);
    g_free(texte);
}

/*===================================================================
                        CALLBACKS - TRANSPOSITION
=======================================================================*/

void on_trans_chiffrer_clicked(GtkWidget *widget, gpointer data) {
    AppWidgets *app = (AppWidgets *)data;
    
    GtkTextBuffer *input_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->trans_input));
    GtkTextBuffer *output_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->trans_output));
    
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(input_buffer, &start, &end);
    char *texte = gtk_text_buffer_get_text(input_buffer, &start, &end, FALSE);
    
    int k = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(app->trans_key_spin));
    
    if (texte && strlen(texte) > 0) {
        char *resultat = chiffrer_transposition(texte, k);
        if (resultat) {
            gtk_text_buffer_set_text(output_buffer, resultat, -1);
            free(resultat);
        }
    }
    
    g_free(texte);
}

void on_trans_dechiffrer_clicked(GtkWidget *widget, gpointer data) {
    AppWidgets *app = (AppWidgets *)data;
    
    GtkTextBuffer *input_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->trans_input));
    GtkTextBuffer *output_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->trans_output));
    
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(input_buffer, &start, &end);
    char *texte = gtk_text_buffer_get_text(input_buffer, &start, &end, FALSE);
    
    int k = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(app->trans_key_spin));
    
    if (texte && strlen(texte) > 0) {
        char *resultat = dechiffrer_transposition(texte, k);
        if (resultat) {
            gtk_text_buffer_set_text(output_buffer, resultat, -1);
            free(resultat);
        }
    }
    
    g_free(texte);
}

void on_trans_attaquer_clicked(GtkWidget *widget, gpointer data) {
    AppWidgets *app = (AppWidgets *)data;
    
    GtkTextBuffer *input_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->trans_input));
    GtkTextBuffer *attack_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->trans_attack_text));
    
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(input_buffer, &start, &end);
    char *texte = gtk_text_buffer_get_text(input_buffer, &start, &end, FALSE);
    
    if (!texte || strlen(texte) == 0) {
        gtk_text_buffer_set_text(attack_buffer, "Aucun texte a attaquer!", -1);
        g_free(texte);
        return;
    }
    
    int len = strlen(texte);
    char *buffer = malloc(len * 100 + 1000);
    strcpy(buffer, "=== ATTAQUE PAR FORCE BRUTE ===\n\n");
    
    int max_k = (len / 2 > 15) ? 15 : len / 2;
    
    for (int k = 2; k <= max_k; k++) {
        char *clair = dechiffrer_transposition(texte, k);
        if (clair) {
            char temp[len + 200];
            snprintf(temp, sizeof(temp), "Cle k = %d:\n%s\n\n", k, clair);
            strcat(buffer, temp);
            free(clair);
        }
    }
    
    gtk_text_buffer_set_text(attack_buffer, buffer, -1);
    
    free(buffer);
    g_free(texte);
}

/*===================================================================
                        CALLBACKS - PLAYFAIR
=======================================================================*/

void on_playfair_afficher_matrice_clicked(GtkWidget *widget, gpointer data) {
    AppWidgets *app = (AppWidgets *)data;
    
    const char *cle = gtk_entry_get_text(GTK_ENTRY(app->playfair_key_entry));
    
    char matrice[5][5];
    creer_matrice_playfair(cle, matrice);
    
    GtkTextBuffer *matrix_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->playfair_matrix_text));
    
    char buffer[500];
    strcpy(buffer, "=== MATRICE PLAYFAIR ===\n\n");
    
    for (int i = 0; i < 5; i++) {
        char line[50] = "    ";
        for (int j = 0; j < 5; j++) {
            char temp[5];
            snprintf(temp, sizeof(temp), "%c  ", matrice[i][j]);
            strcat(line, temp);
        }
        strcat(line, "\n");
        strcat(buffer, line);
    }
    
    gtk_text_buffer_set_text(matrix_buffer, buffer, -1);
}

void on_playfair_chiffrer_clicked(GtkWidget *widget, gpointer data) {
    AppWidgets *app = (AppWidgets *)data;
    
    GtkTextBuffer *input_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->playfair_input));
    GtkTextBuffer *output_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->playfair_output));
    
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(input_buffer, &start, &end);
    char *texte = gtk_text_buffer_get_text(input_buffer, &start, &end, FALSE);
    
    const char *cle = gtk_entry_get_text(GTK_ENTRY(app->playfair_key_entry));
    
    if (texte && strlen(texte) > 0) {
        char matrice[5][5];
        creer_matrice_playfair(cle, matrice);
        
        char *resultat = chiffrer_playfair(texte, matrice);
        if (resultat) {
            gtk_text_buffer_set_text(output_buffer, resultat, -1);
            free(resultat);
        }
    }
    
    g_free(texte);
}

void on_playfair_dechiffrer_clicked(GtkWidget *widget, gpointer data) {
    AppWidgets *app = (AppWidgets *)data;
    
    GtkTextBuffer *input_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->playfair_input));
    GtkTextBuffer *output_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->playfair_output));
    
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(input_buffer, &start, &end);
    char *texte = gtk_text_buffer_get_text(input_buffer, &start, &end, FALSE);
    
    const char *cle = gtk_entry_get_text(GTK_ENTRY(app->playfair_key_entry));
    
    if (texte && strlen(texte) > 0) {
        char matrice[5][5];
        creer_matrice_playfair(cle, matrice);
        
        char *resultat = dechiffrer_playfair(texte, matrice);
        if (resultat) {
            gtk_text_buffer_set_text(output_buffer, resultat, -1);
            free(resultat);
        }
    }
    
    g_free(texte);
}

/*===================================================================
                        CALLBACKS - GENERATEUR PLAYFAIR
=======================================================================*/

void on_generer_defi_clicked(GtkWidget *widget, gpointer data) {
    AppWidgets *app = (AppWidgets *)data;
    
    const char *cle = gtk_entry_get_text(GTK_ENTRY(app->gen_key_entry));
    int n = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(app->gen_n_spin));
    const char *flag = gtk_entry_get_text(GTK_ENTRY(app->gen_flag_entry));
    
    char matrice[5][5];
    creer_matrice_playfair(cle, matrice);
    
    GtkTextBuffer *output_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->gen_output));
    
    char *buffer = malloc(10000);
    strcpy(buffer, "+========================================================+\n");
    strcat(buffer, "|          DEFI PLAYFAIR GENERE                          |\n");
    strcat(buffer, "+========================================================+\n\n");
    strcat(buffer, "Matrice secrete creee avec votre cle.\n\n");
    
    // First, encrypt the flag to get the cryptogram
    char *flag_chiffre = chiffrer_playfair(flag, matrice);
    
    // Prepare the plaintext version of the flag
    char *flag_prepare = strdup(flag);
    preparer_texte_playfair(flag_prepare);
    int flag_len = strlen(flag_prepare);
    if (flag_len % 2 == 1) {
        flag_prepare = realloc(flag_prepare, flag_len + 2);
        flag_prepare[flag_len] = 'X';
        flag_prepare[flag_len + 1] = '\0';
        flag_len++;
    }
    
    // Extract ALL bigrams from the plaintext flag
    int n_total_bigrams = flag_len / 2;
    char all_bigrams[50][3];
    char all_chiffres[50][3];
    
    for (int i = 0; i < n_total_bigrams; i++) {
        all_bigrams[i][0] = flag_prepare[i*2];
        all_bigrams[i][1] = flag_prepare[i*2 + 1];
        all_bigrams[i][2] = '\0';
        chiffrer_bigramme_playfair(all_bigrams[i], matrice, all_chiffres[i]);
        all_chiffres[i][2] = '\0';
    }
    
    // Shuffle the bigrams randomly (Fisher-Yates)
    for (int i = n_total_bigrams - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        // Swap bigrams
        char temp_bg[3], temp_ch[3];
        strcpy(temp_bg, all_bigrams[i]);
        strcpy(temp_ch, all_chiffres[i]);
        strcpy(all_bigrams[i], all_bigrams[j]);
        strcpy(all_chiffres[i], all_chiffres[j]);
        strcpy(all_bigrams[j], temp_bg);
        strcpy(all_chiffres[j], temp_ch);
    }
    
    // Determine how many pairs to show
    int n_pairs_to_show = (n < n_total_bigrams) ? n : n_total_bigrams;
    
    strcat(buffer, "=== INDICES (Paires Clair -> Chiffre) ===\n");
    strcat(buffer, "(Paires extraites aleatoirement du message)\n\n");
    
    for (int i = 0; i < n_pairs_to_show; i++) {
        char line[100];
        snprintf(line, sizeof(line), "  %s -> %s\n", all_bigrams[i], all_chiffres[i]);
        strcat(buffer, line);
    }
    
    // If we need MORE pairs than available in the message, add random ones
    if (n > n_total_bigrams) {
        const char *alphabet = "ABCDEFGHIKLMNOPQRSTUVXYZ"; // 25 letters (no W)
        int alpha_len = strlen(alphabet);
        int pairs_needed = n - n_total_bigrams;
        int pairs_added = 0;
        
        for (int attempt = 0; attempt < 100 && pairs_added < pairs_needed; attempt++) {
            // Generate random bigram
            char random_bg[3];
            random_bg[0] = alphabet[rand() % alpha_len];
            random_bg[1] = alphabet[rand() % alpha_len];
            random_bg[2] = '\0';
            
            // Check if already used
            int already_used = 0;
            for (int i = 0; i < n_pairs_to_show; i++) {
                if (strcmp(all_bigrams[i], random_bg) == 0) {
                    already_used = 1;
                    break;
                }
            }
            
            if (!already_used) {
                char chiffre[3] = {0};
                chiffrer_bigramme_playfair(random_bg, matrice, chiffre);
                chiffre[2] = '\0';
                
                char line[100];
                snprintf(line, sizeof(line), "  %s -> %s\n", random_bg, chiffre);
                strcat(buffer, line);
                pairs_added++;
            }
        }
    }
    
    strcat(buffer, "\n=== CRYPTOGRAMME A DECHIFFRER (FLAG) ===\n\n");
    strcat(buffer, "  ");
    strcat(buffer, flag_chiffre);
    strcat(buffer, "\n\n");
    strcat(buffer, "DEBUG - Texte clair prepare: ");
    strcat(buffer, flag_prepare);
    strcat(buffer, "\n\n+========================================================+\n");
    strcat(buffer, "|  Copiez les paires dans l'onglet Solveur Playfair!    |\n");
    strcat(buffer, "+========================================================+\n");
    
    gtk_text_buffer_set_text(output_buffer, buffer, -1);
    
    free(buffer);
    free(flag_chiffre);
    free(flag_prepare);
}

/*===================================================================
                        CALLBACKS - SOLVEUR PLAYFAIR
=======================================================================*/

void on_resoudre_defi_clicked(GtkWidget *widget, gpointer data) {
    AppWidgets *app = (AppWidgets *)data;
    
    GtkTextBuffer *paires_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->solver_paires_text));
    GtkTextBuffer *output_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->solver_output));
    
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(paires_buffer, &start, &end);
    char *paires_text = gtk_text_buffer_get_text(paires_buffer, &start, &end, FALSE);
    
    const char *cryptogramme = gtk_entry_get_text(GTK_ENTRY(app->solver_cryptogramme_entry));
    
    if (!paires_text || strlen(paires_text) == 0) {
        gtk_text_buffer_set_text(output_buffer, 
            "X Erreur: Aucune paire fournie!\n\nFormat attendu:\nTH -> HM\nHE -> DF\n...", -1);
        if (paires_text) g_free(paires_text);
        return;
    }
    
    char *buffer = malloc(100000);
    strcpy(buffer, "+========================================================+\n");
    strcat(buffer, "|     CRYPTANALYSE PLAYFAIR AUTOMATIQUE                  |\n");
    strcat(buffer, "+========================================================+\n\n");
    
    char *paires_copy = strdup(paires_text);
    char *line = strtok(paires_copy, "\n");
    int paire_count = 0;
    
    PaireAnalysee paires[50];
    
    strcat(buffer, "ETAPE 1: Extraction des paires\n");
    strcat(buffer, "------------------------------------------------\n");
    
    while (line != NULL && paire_count < 50) {
        char clair[3], chiffre[3];
        if (sscanf(line, "%2s -> %2s", clair, chiffre) == 2 || 
            sscanf(line, "%2s->%2s", clair, chiffre) == 2 ||
            sscanf(line, "%2s %2s", clair, chiffre) == 2) {
            
            for (int i = 0; i < 2; i++) {
                clair[i] = toupper(clair[i]);
                chiffre[i] = toupper(chiffre[i]);
                if (clair[i] == 'W') clair[i] = 'V';
                if (chiffre[i] == 'W') chiffre[i] = 'V';
            }
            
            strcpy(paires[paire_count].clair, clair);
            strcpy(paires[paire_count].chiffre, chiffre);
            paires[paire_count].type = 0;
            
            char temp[100];
            snprintf(temp, sizeof(temp), "  OK Paire %2d: %s -> %s\n", 
                    paire_count + 1, clair, chiffre);
            strcat(buffer, temp);
            
            paire_count++;
        }
        line = strtok(NULL, "\n");
    }
    
    free(paires_copy);
    
    char stats[200];
    snprintf(stats, sizeof(stats), "\nTotal: %d paires detectees\n\n", paire_count);
    strcat(buffer, stats);
    
    if (paire_count < 3) {
        strcat(buffer, "X ERREUR: Nombre de paires insuffisant!\n");
        strcat(buffer, "   Minimum: 3-5 paires\n");
        strcat(buffer, "   Recommande: 8-10 paires\n");
        gtk_text_buffer_set_text(output_buffer, buffer, -1);
        free(buffer);
        g_free(paires_text);
        return;
    }
    
    strcat(buffer, "ETAPE 2: Generation de matrices\n");
    strcat(buffer, "------------------------------------------------\n");
    
    gtk_text_buffer_set_text(output_buffer, buffer, -1);
    while (gtk_events_pending()) gtk_main_iteration();
    
    char matrices_possibles[20][5][5];
    int n_matrices = 0;
    
    generer_matrices_candidates(paires, paire_count, matrices_possibles, &n_matrices, 10);
    
    snprintf(stats, sizeof(stats), "  OK %d matrices generees\n\n", n_matrices);
    strcat(buffer, stats);
    
    strcat(buffer, "ETAPE 3: Optimisation (cela peut prendre quelques secondes)\n");
    strcat(buffer, "------------------------------------------------\n");
    
    gtk_text_buffer_set_text(output_buffer, buffer, -1);
    while (gtk_events_pending()) gtk_main_iteration();
    
    int meilleure_idx = 0;
    int meilleur_score = 0;
    // Increase iterations significantly based on number of pairs
    int iterations = (paire_count < 5) ? 15000 : (paire_count < 8) ? 25000 : 35000;
    
    strcat(buffer, "\nTest des matrices initiales:\n");
    for (int m = 0; m < n_matrices; m++) {
        int score_initial = tester_matrice_avec_paires(matrices_possibles[m], paires, paire_count);
        char temp[100];
        snprintf(temp, sizeof(temp), "  Matrice %d: score initial = %d/%d\n", m+1, score_initial, paire_count);
        strcat(buffer, temp);
    }
    strcat(buffer, "\nOptimisation en cours: ");
    gtk_text_buffer_set_text(output_buffer, buffer, -1);
    while (gtk_events_pending()) gtk_main_iteration();
    
    for (int m = 0; m < n_matrices; m++) {
        int score_avant = tester_matrice_avec_paires(matrices_possibles[m], paires, paire_count);
        optimiser_matrice_avancee(matrices_possibles[m], paires, paire_count, iterations);
        
        int score = tester_matrice_avec_paires(matrices_possibles[m], paires, paire_count);
        
        char temp[150];
        snprintf(temp, sizeof(temp), "\n  Matrice %d: %d->%d/%d ", m+1, score_avant, score, paire_count);
        strcat(buffer, temp);
        
        if (score > meilleur_score) {
            meilleur_score = score;
            meilleure_idx = m;
            strcat(buffer, "[MEILLEURE]");
        }
        
        gtk_text_buffer_set_text(output_buffer, buffer, -1);
        while (gtk_events_pending()) gtk_main_iteration();
    }
    
    strcat(buffer, " OK\n\n");
    
    strcat(buffer, "ETAPE 4: Resultats\n");
    strcat(buffer, "------------------------------------------------\n");
    
    int affichees = 0;
    for (int m = 0; m < n_matrices && affichees < 3; m++) {
        int score = tester_matrice_avec_paires(matrices_possibles[m], paires, paire_count);
        
        if (score >= meilleur_score - 2 || affichees == 0) {
            double pourcentage = (score * 100.0) / paire_count;
            
            char eval[300];
            snprintf(eval, sizeof(eval), "\n%s Matrice #%d - Score: %d/%d (%.1f%%)\n", 
                    (m == meilleure_idx) ? "***" : "   ",
                    affichees + 1, score, paire_count, pourcentage);
            strcat(buffer, eval);
            
            strcat(buffer, "  +-------------+\n");
            for (int i = 0; i < 5; i++) {
                strcat(buffer, "  | ");
                for (int j = 0; j < 5; j++) {
                    char c[4];
                    snprintf(c, sizeof(c), "%c ", matrices_possibles[m][i][j]);
                    strcat(buffer, c);
                }
                strcat(buffer, "|\n");
            }
            strcat(buffer, "  +-------------+\n");
            
            if (m == meilleure_idx) {
                if (pourcentage == 100.0) {
                    strcat(buffer, "  PARFAIT!\n");
                } else if (pourcentage >= 80.0) {
                    strcat(buffer, "  OK Excellent\n");
                } else if (pourcentage >= 60.0) {
                    strcat(buffer, "  ! Bonne approximation\n");
                } else {
                    strcat(buffer, "  ! Partiel\n");
                }
            }
            
            affichees++;
        }
    }
    
    if (strlen(cryptogramme) > 0) {
        strcat(buffer, "\nETAPE 5: Dechiffrement\n");
        strcat(buffer, "------------------------------------------------\n");
        
        char crypto_clean[1000];
        strcpy(crypto_clean, cryptogramme);
        preparer_texte_playfair(crypto_clean);
        
        snprintf(stats, sizeof(stats), "  Cryptogramme: %s\n\n", crypto_clean);
        strcat(buffer, stats);
        
        char *dechiffre = dechiffrer_playfair(crypto_clean, matrices_possibles[meilleure_idx]);
        
        strcat(buffer, "  Message dechiffre:\n");
        strcat(buffer, "  +==========================================+\n");
        strcat(buffer, "  |  ");
        strcat(buffer, dechiffre);
        
        int len = strlen(dechiffre);
        for (int i = len; i < 40; i++) strcat(buffer, " ");
        strcat(buffer, "|\n");
        strcat(buffer, "  +==========================================+\n");
        
        free(dechiffre);
        
        double pourcentage = (meilleur_score * 100.0) / paire_count;
        
        if (pourcentage == 100.0) {
            strcat(buffer, "\n  OK SUCCES TOTAL! Message fiable.\n");
        } else if (pourcentage >= 80.0) {
            strcat(buffer, "\n  OK Tres probable que le message soit correct.\n");
        } else if (pourcentage >= 60.0) {
            strcat(buffer, "\n  ! Peut contenir des erreurs.\n");
        } else {
            strcat(buffer, "\n  X Ajoutez plus de paires pour ameliorer.\n");
        }
    }
    
    strcat(buffer, "\n+========================================================+\n");
    strcat(buffer, "|                 FIN DE L'ANALYSE                       |\n");
    strcat(buffer, "+========================================================+\n");
    
    gtk_text_buffer_set_text(output_buffer, buffer, -1);
    
    free(buffer);
    g_free(paires_text);
}

/*===================================================================
                        CREATION DE L'INTERFACE
=======================================================================*/

GtkWidget *create_cesar_tab(AppWidgets *app) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);
    
    GtkWidget *label1 = gtk_label_new("Texte a traiter:");
    gtk_box_pack_start(GTK_BOX(vbox), label1, FALSE, FALSE, 0);
    
    GtkWidget *scrolled1 = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled1), 
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scrolled1, -1, 100);
    app->cesar_input = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app->cesar_input), GTK_WRAP_WORD);
    gtk_container_add(GTK_CONTAINER(scrolled1), app->cesar_input);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled1, FALSE, FALSE, 0);
    
    GtkWidget *hbox_key = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *label_key = gtk_label_new("Cle (k):");
    app->cesar_key_spin = gtk_spin_button_new_with_range(0, 255, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(app->cesar_key_spin), 3);
    gtk_box_pack_start(GTK_BOX(hbox_key), label_key, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_key), app->cesar_key_spin, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_key, FALSE, FALSE, 0);
    
    GtkWidget *hbox_buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *btn_chiffrer = gtk_button_new_with_label("Chiffrer");
    GtkWidget *btn_dechiffrer = gtk_button_new_with_label("Dechiffrer");
    GtkWidget *btn_analyser = gtk_button_new_with_label("Analyser");
    
    g_signal_connect(btn_chiffrer, "clicked", G_CALLBACK(on_cesar_chiffrer_clicked), app);
    g_signal_connect(btn_dechiffrer, "clicked", G_CALLBACK(on_cesar_dechiffrer_clicked), app);
    g_signal_connect(btn_analyser, "clicked", G_CALLBACK(on_cesar_analyser_clicked), app);
    
    gtk_box_pack_start(GTK_BOX(hbox_buttons), btn_chiffrer, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_buttons), btn_dechiffrer, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_buttons), btn_analyser, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_buttons, FALSE, FALSE, 0);
    
    GtkWidget *label2 = gtk_label_new("Resultat:");
    gtk_box_pack_start(GTK_BOX(vbox), label2, FALSE, FALSE, 0);
    
    GtkWidget *scrolled2 = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled2), 
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scrolled2, -1, 100);
    app->cesar_output = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app->cesar_output), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app->cesar_output), GTK_WRAP_WORD);
    gtk_container_add(GTK_CONTAINER(scrolled2), app->cesar_output);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled2, FALSE, FALSE, 0);
    
    GtkWidget *label3 = gtk_label_new("Analyse de frequence:");
    gtk_box_pack_start(GTK_BOX(vbox), label3, FALSE, FALSE, 0);
    
    GtkWidget *scrolled3 = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled3), 
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    app->cesar_freq_text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app->cesar_freq_text), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app->cesar_freq_text), GTK_WRAP_WORD);
    gtk_container_add(GTK_CONTAINER(scrolled3), app->cesar_freq_text);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled3, TRUE, TRUE, 0);
    
    return vbox;
}

GtkWidget *create_transposition_tab(AppWidgets *app) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);
    
    GtkWidget *label1 = gtk_label_new("Texte a traiter:");
    gtk_box_pack_start(GTK_BOX(vbox), label1, FALSE, FALSE, 0);
    
    GtkWidget *scrolled1 = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled1), 
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scrolled1, -1, 100);
    app->trans_input = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app->trans_input), GTK_WRAP_WORD);
    gtk_container_add(GTK_CONTAINER(scrolled1), app->trans_input);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled1, FALSE, FALSE, 0);
    
    GtkWidget *hbox_key = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *label_key = gtk_label_new("Cle (k):");
    app->trans_key_spin = gtk_spin_button_new_with_range(1, 100, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(app->trans_key_spin), 3);
    gtk_box_pack_start(GTK_BOX(hbox_key), label_key, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_key), app->trans_key_spin, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_key, FALSE, FALSE, 0);
    
    GtkWidget *hbox_buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *btn_chiffrer = gtk_button_new_with_label("Chiffrer");
    GtkWidget *btn_dechiffrer = gtk_button_new_with_label("Dechiffrer");
    GtkWidget *btn_attaquer = gtk_button_new_with_label("Attaquer");
    
    g_signal_connect(btn_chiffrer, "clicked", G_CALLBACK(on_trans_chiffrer_clicked), app);
    g_signal_connect(btn_dechiffrer, "clicked", G_CALLBACK(on_trans_dechiffrer_clicked), app);
    g_signal_connect(btn_attaquer, "clicked", G_CALLBACK(on_trans_attaquer_clicked), app);
    
    gtk_box_pack_start(GTK_BOX(hbox_buttons), btn_chiffrer, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_buttons), btn_dechiffrer, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_buttons), btn_attaquer, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_buttons, FALSE, FALSE, 0);
    
    GtkWidget *label2 = gtk_label_new("Resultat:");
    gtk_box_pack_start(GTK_BOX(vbox), label2, FALSE, FALSE, 0);
    
    GtkWidget *scrolled2 = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled2), 
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scrolled2, -1, 100);
    app->trans_output = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app->trans_output), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app->trans_output), GTK_WRAP_WORD);
    gtk_container_add(GTK_CONTAINER(scrolled2), app->trans_output);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled2, FALSE, FALSE, 0);
    
    GtkWidget *label3 = gtk_label_new("Resultats de l'attaque:");
    gtk_box_pack_start(GTK_BOX(vbox), label3, FALSE, FALSE, 0);
    
    GtkWidget *scrolled3 = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled3), 
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    app->trans_attack_text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app->trans_attack_text), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app->trans_attack_text), GTK_WRAP_WORD);
    gtk_container_add(GTK_CONTAINER(scrolled3), app->trans_attack_text);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled3, TRUE, TRUE, 0);
    
    return vbox;
}

GtkWidget *create_playfair_tab(AppWidgets *app) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);
    
    GtkWidget *hbox_key = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *label_key = gtk_label_new("Mot-cle:");
    app->playfair_key_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(app->playfair_key_entry), "SECRET");
    gtk_box_pack_start(GTK_BOX(hbox_key), label_key, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_key), app->playfair_key_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_key, FALSE, FALSE, 0);
    
    GtkWidget *btn_matrix = gtk_button_new_with_label("Afficher la Matrice");
    g_signal_connect(btn_matrix, "clicked", G_CALLBACK(on_playfair_afficher_matrice_clicked), app);
    gtk_box_pack_start(GTK_BOX(vbox), btn_matrix, FALSE, FALSE, 0);
    
    GtkWidget *scrolled_matrix = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_matrix), 
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scrolled_matrix, -1, 120);
    app->playfair_matrix_text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app->playfair_matrix_text), FALSE);
    
    GtkCssProvider *css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider,
        "textview { font-family: monospace; font-size: 12pt; }",
        -1, NULL);
    GtkStyleContext *context = gtk_widget_get_style_context(app->playfair_matrix_text);
    gtk_style_context_add_provider(context,
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(css_provider);
    
    gtk_container_add(GTK_CONTAINER(scrolled_matrix), app->playfair_matrix_text);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_matrix, FALSE, FALSE, 0);
    
    GtkWidget *label1 = gtk_label_new("Texte a traiter:");
    gtk_box_pack_start(GTK_BOX(vbox), label1, FALSE, FALSE, 0);
    
    GtkWidget *scrolled1 = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled1), 
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scrolled1, -1, 80);
    app->playfair_input = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app->playfair_input), GTK_WRAP_WORD);
    gtk_container_add(GTK_CONTAINER(scrolled1), app->playfair_input);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled1, FALSE, FALSE, 0);
    
    GtkWidget *hbox_buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *btn_chiffrer = gtk_button_new_with_label("Chiffrer");
    GtkWidget *btn_dechiffrer = gtk_button_new_with_label("Dechiffrer");
    
    g_signal_connect(btn_chiffrer, "clicked", G_CALLBACK(on_playfair_chiffrer_clicked), app);
    g_signal_connect(btn_dechiffrer, "clicked", G_CALLBACK(on_playfair_dechiffrer_clicked), app);
    
    gtk_box_pack_start(GTK_BOX(hbox_buttons), btn_chiffrer, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_buttons), btn_dechiffrer, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_buttons, FALSE, FALSE, 0);
    
    GtkWidget *label2 = gtk_label_new("Resultat:");
    gtk_box_pack_start(GTK_BOX(vbox), label2, FALSE, FALSE, 0);
    
    GtkWidget *scrolled2 = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled2), 
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    app->playfair_output = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app->playfair_output), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app->playfair_output), GTK_WRAP_WORD);
    gtk_container_add(GTK_CONTAINER(scrolled2), app->playfair_output);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled2, TRUE, TRUE, 0);
    
    return vbox;
}

GtkWidget *create_generateur_tab(AppWidgets *app) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);
    
    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), "<b><big>Generateur de Defis Playfair</big></b>");
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 10);
    
    GtkWidget *hbox_key = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *label_key = gtk_label_new("Mot-cle Secret:");
    app->gen_key_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(app->gen_key_entry), "PLAYFAIR");
    gtk_box_pack_start(GTK_BOX(hbox_key), label_key, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_key), app->gen_key_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_key, FALSE, FALSE, 0);
    
    GtkWidget *hbox_n = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *label_n = gtk_label_new("Nombre de paires d'indices:");
    app->gen_n_spin = gtk_spin_button_new_with_range(1, 20, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(app->gen_n_spin), 5);
    gtk_box_pack_start(GTK_BOX(hbox_n), label_n, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_n), app->gen_n_spin, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_n, FALSE, FALSE, 0);
    
    GtkWidget *hbox_flag = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *label_flag = gtk_label_new("Message Secret (Flag):");
    app->gen_flag_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(app->gen_flag_entry), "TROUVEMOI");
    gtk_box_pack_start(GTK_BOX(hbox_flag), label_flag, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_flag), app->gen_flag_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_flag, FALSE, FALSE, 0);
    
    GtkWidget *btn_generer = gtk_button_new_with_label("Generer le Defi");
    g_signal_connect(btn_generer, "clicked", G_CALLBACK(on_generer_defi_clicked), app);
    gtk_box_pack_start(GTK_BOX(vbox), btn_generer, FALSE, FALSE, 10);
    
    GtkWidget *label_output = gtk_label_new("Defi Genere:");
    gtk_box_pack_start(GTK_BOX(vbox), label_output, FALSE, FALSE, 0);
    
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), 
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    app->gen_output = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app->gen_output), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app->gen_output), GTK_WRAP_WORD);
    gtk_container_add(GTK_CONTAINER(scrolled), app->gen_output);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);
    
    return vbox;
}

GtkWidget *create_solveur_tab(AppWidgets *app) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);
    
    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), "<b><big>Solveur de Defis Playfair</big></b>");
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 10);
    
    GtkWidget *label1 = gtk_label_new("Paires connues (format: TH -> HM):");
    gtk_box_pack_start(GTK_BOX(vbox), label1, FALSE, FALSE, 0);
    
    GtkWidget *scrolled1 = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled1), 
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scrolled1, -1, 150);
    app->solver_paires_text = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app->solver_paires_text), GTK_WRAP_WORD);
    gtk_container_add(GTK_CONTAINER(scrolled1), app->solver_paires_text);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled1, FALSE, FALSE, 0);
    
    GtkWidget *hbox_crypto = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *label_crypto = gtk_label_new("Cryptogramme:");
    app->solver_cryptogramme_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hbox_crypto), label_crypto, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_crypto), app->solver_cryptogramme_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_crypto, FALSE, FALSE, 0);
    
    GtkWidget *btn_resoudre = gtk_button_new_with_label("Resoudre le Defi");
    g_signal_connect(btn_resoudre, "clicked", G_CALLBACK(on_resoudre_defi_clicked), app);
    gtk_box_pack_start(GTK_BOX(vbox), btn_resoudre, FALSE, FALSE, 10);
    
    GtkWidget *label_output = gtk_label_new("Analyse et Solutions:");
    gtk_box_pack_start(GTK_BOX(vbox), label_output, FALSE, FALSE, 0);
    
    GtkWidget *scrolled2 = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled2), 
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    app->solver_output = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app->solver_output), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app->solver_output), GTK_WRAP_WORD);
    gtk_container_add(GTK_CONTAINER(scrolled2), app->solver_output);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled2, TRUE, TRUE, 0);
    
    return vbox;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    
    srand(time(NULL));
    
    AppWidgets app;
    
    app.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app.window), "CryptoLab - Cryptographie Classique");
    gtk_window_set_default_size(GTK_WINDOW(app.window), 900, 700);
    gtk_container_set_border_width(GTK_CONTAINER(app.window), 10);
    g_signal_connect(app.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    app.notebook = gtk_notebook_new();
    
    GtkWidget *cesar_tab = create_cesar_tab(&app);
    GtkWidget *trans_tab = create_transposition_tab(&app);
    GtkWidget *playfair_tab = create_playfair_tab(&app);
    GtkWidget *generateur_tab = create_generateur_tab(&app);
    GtkWidget *solveur_tab = create_solveur_tab(&app);
    
    gtk_notebook_append_page(GTK_NOTEBOOK(app.notebook), cesar_tab, 
                            gtk_label_new("Cesar"));
    gtk_notebook_append_page(GTK_NOTEBOOK(app.notebook), trans_tab, 
                            gtk_label_new("Transposition"));
    gtk_notebook_append_page(GTK_NOTEBOOK(app.notebook), playfair_tab, 
                            gtk_label_new("Playfair"));
    gtk_notebook_append_page(GTK_NOTEBOOK(app.notebook), generateur_tab, 
                            gtk_label_new("Generateur"));
    gtk_notebook_append_page(GTK_NOTEBOOK(app.notebook), solveur_tab, 
                            gtk_label_new("Solveur"));
    
    gtk_container_add(GTK_CONTAINER(app.window), app.notebook);
    
    gtk_widget_show_all(app.window);
    
    gtk_main();
    
    return 0;
}