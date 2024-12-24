#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// sembol tablosu için 
typedef struct {
    char name[50];
    char type[10];
    int isOccupied;
} SymbolTableEntry;

int m, m2; //hash tablosunun boyutlarý
SymbolTableEntry* symbolTable;

// Ýlk hash fonksiyonu
int h1(int key) {
    return key % m;
}

// Ýkinci hash fonksiyonu
int h2(int key) {
    return 1 + (key % m2);
}

// double hashing
int h(int key, int i) {
    return (h1(key) + i * h2(key)) % m;
}

// String'i sayýsal deðere dönüþtüren fonksiyon
int horner(const char* str) {
	int i;
    int result = 0;
    for (i = 0; str[i] != '\0'; i++) {
        result = result * 31 + str[i];
    }
    return result;
}

// Deðiþken sayýsýnýn 2 katýndan büyük en küçük asal sayýyý bulan fonksiyon
int nextPrime(int count) {
    int n = 2 * count + 1; // baþlangýç deðeri
    int isPrime = 0; // ilk durumda asal olmadýðýný varsay
    int i;
    while (!isPrime) { 
        isPrime = 1; 
        for (i = 2; i * i <= n && isPrime; i++) {
            if (n % i == 0) {
                isPrime = 0; // asal deðil, tekrar kontrol et
            }
        }
        if (!isPrime) {
            n++; 
        }
    }
    return n; 
}

// Yeni deðiþkeni hash tablosuna ekleyen fonksiyon
void insert(const char* name, const char* type, int isDebug) {
    int key = horner(name);
    int i, index;
    int firstIndex = h1(key); // Ýlk hesaplanan hash adresi

    for (i = 0; i < m; i++) {
        index = h(key, i);
        if (!symbolTable[index].isOccupied) {
            // Boþ bir alan bulundu, deðiþkeni yerleþtir
            strcpy(symbolTable[index].name, name);
            strcpy(symbolTable[index].type, type);
            symbolTable[index].isOccupied = 1;
            if (isDebug) {
                printf("Degisken: %s, Tipi: %s\n", name, type);
                printf("  Ilk Hesaplanan Adres: %d\n", firstIndex);
                printf("  Yerlesilen Adres: %d\n", index);
            }
            return;
        } else if (strcmp(symbolTable[index].name, name) == 0) {
            // Ayný isimde bir deðiþken zaten mevcutsa
            if (strcmp(symbolTable[index].type, type) != 0) {
                printf("HATA: Degisken %s farkli tipte (%s) zaten tanimlanmis.\n", name, symbolTable[index].type);
                return;
            } else {
                printf("HATA: Degisken %s onceden tanimlanmis.\n", name);
                return;
            }
        }
    }

    // Eðer döngü tamamlanýrsa tablo dolmuþ demektir
    printf("HATA: Tabloda yer bulunamadi.\n");
}



// Hash tablosunda o deðiþkenin olup olmadýðýný kontrol eder
void lookup(const char* name) {
	int i;
    int key = horner(name);
    for (i = 0; i < m; i++) {
        int index = h(key, i);
        if (symbolTable[index].isOccupied && strcmp(symbolTable[index].name, name) == 0) {
            return;
        }
    }
    printf("HATA: Degisken %s tanimlanmamis.\n", name);
}

// Dosyayý okur ve deðiþkenleri tabloya ekler
void parseFile(const char* filename, int isDebug) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("HATA: Dosya acilamadi %s\n", filename);
        return;
    }

    char line[256];
    char declaredVariables[100][50]; // Benzersiz deðiþken isimlerini saklar
    char declaredTypes[100][10];    // Benzersiz deðiþkenlerin tiplerini saklar
    int uniqueCount = 0;
    int i, isDuplicate;

    // Deðiþken sayýsýný hesapla
    while (fgets(line, sizeof(line), file)) {
    	char* token = strtok(line, " ;,\n");
    	char type[10] = "";
    	while (token) {
        	if (strcmp(token, "int") == 0 || strcmp(token, "float") == 0 || strcmp(token, "char") == 0) {
            	strcpy(type, token);
        	} else if (type[0] != '\0' && token[0] == '_') {
            	// Daha önce eklenmiþ mi kontrol et
            	isDuplicate = 0;
            	for (i = 0; i < uniqueCount && !isDuplicate; i++) {
                	if (strcmp(declaredVariables[i], token) == 0) { // eðer dizideki bir string ile aynýysa
                    	isDuplicate = 1;
                	}
            	}

            	// Eðer benzersizse, listeye ekle
            	if (!isDuplicate) {
                	strcpy(declaredVariables[uniqueCount], token);
                	strcpy(declaredTypes[uniqueCount], type);
                	uniqueCount++;
            	}
        	}
        	token = strtok(NULL, " ;,\n");
    	}
	}


    // Benzersiz deðiþken sayýsý kadar tablo boyutu belirle
    m = nextPrime(uniqueCount);
    m2 = m - 3;

    symbolTable = (SymbolTableEntry*)calloc(m, sizeof(SymbolTableEntry));

    // Dosyanýn baþýna dön
    rewind(file);

    // Deðiþkenleri tabloya ekle
    while (fgets(line, sizeof(line), file)) {
        char* token = strtok(line, " ;,\n");  // okunan satýrý karakterlere göre parçala
        char type[10] = "";
        while (token) {
            if (strcmp(token, "int") == 0 || strcmp(token, "float") == 0 || strcmp(token, "char") == 0) {
                strcpy(type, token);  // geçerli veri tipi kontrolü
            } else if (type[0] != '\0' && token[0] == '_') { // type boþ deðilse ve deðiþken _ ile baþlýyorsa 
                insert(token, type, isDebug);
            } else if (token[0] == '_') { // type boþsa ve deðiþken _ ile baþlýyorsa deðiþken önceden tanýmlanmýþ mý
                lookup(token);
            }
            token = strtok(NULL, " ;,\n"); // bir sonraki parçaya geç
        }
    }

    fclose(file);

    // Debug modu açýkken tabloyu yazdýr
    if (isDebug) {
        printf("Tanimlanmis degisken sayisi: %d\n", uniqueCount);
        printf("Sembol tablosunun boyutu: %d\n", m);
        for (i = 0; i < m; i++) {
            if (symbolTable[i].isOccupied) {
                printf("Index %d: %s (%s)\n", i, symbolTable[i].name, symbolTable[i].type);
            }
        }
    }
}



int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Kullanim: %s <NORMAL|DEBUG>\n", argv[0]);
        return 1;
    }

    const char* filename = "test.c";
    int isDebug = strcmp(argv[1], "DEBUG") == 0;

    parseFile(filename, isDebug);

    free(symbolTable);
    return 0;
}

