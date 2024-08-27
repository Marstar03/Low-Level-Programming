#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

int IsPalindrome(char word[]) {
    printf("%s\n", word);
    bool ispal = true;

    int count = 0;

    // Remove any spaces from string
    for (int i = 0; i < (int) strlen(word); i++) {
        if (word[i] != ' ') {
            word[count] = word[i];
            count++;
        }
    }
    word[count] = '\0';

    printf("%s\n", word);

    // Check if word is palindrome
    for (int i = 0; i < (int) (strlen(word)/2); i++) {
        char first = tolower(word[i]);
        char second = tolower(word[strlen(word) -1 - i]);
        printf("first %c, second %c\n", first, second);

        if (first != second && first != '?' && second != '?') {
            ispal = false;
        }
    }


    return ispal;
}




int main() {
    char word[] = "Grav ned den varg";
    printf("%d\n", IsPalindrome(word));
    return 0;
}