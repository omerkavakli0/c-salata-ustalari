#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdbool.h>

#define NUM_USTALAR 6
#define MAX_LINE 100

typedef struct {
    char malzeme1;
    char malzeme2;
} MalzemeCifti;

typedef struct {
    int id;
    char sahip1;
    char sahip2;
} Usta;

MalzemeCifti *malzeme_listesi = NULL;
int toplam_satir = 0;
int mevcut_index = 0;

sem_t usta_sem[NUM_USTALAR];
sem_t toptanci_sem;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

Usta ustalar[NUM_USTALAR] = {
    {0, 'Y', 'T'},
    {1, 'Y', 'S'},
    {2, 'Y', 'L'},
    {3, 'T', 'S'},
    {4, 'T', 'L'},
    {5, 'S', 'L'},
};

// Malzeme harfini stringe çeviren yardımcı fonksiyon
const char* malzeme_adi(char m) {
    switch (m) {
        case 'Y': return "Yağ";
        case 'T': return "Tuz";
        case 'L': return "Limon";
        case 'S': return "Sebze";
        default: return "Bilinmeyen";
    }
}

// Ustanın sahip olduğu ve olmadığı malzemeleri yazdıran fonksiyon
void usta_bilgi_yazdir(int id, char sahip1, char sahip2) {
    printf("Usta %d: ", id + 1);
    printf("%s ve %s var, ", malzeme_adi(sahip1), malzeme_adi(sahip2));
    // Sahip olunmayanları bul
    char tum_malzeme[] = {'Y', 'T', 'L', 'S'};
    int yazildi = 0;
    for (int i = 0; i < 4; ++i) {
        if (tum_malzeme[i] != sahip1 && tum_malzeme[i] != sahip2) {
            if (yazildi == 0)
                printf("%s", malzeme_adi(tum_malzeme[i]));
            else
                printf(" ve %s", malzeme_adi(tum_malzeme[i]));
            yazildi++;
        }
    }
    printf(" yok\n");
}

// KURAL: Tüm salata ustaları, aynı fonksiyonu kullanarak çalışmalı; her biri için ayrı işlev tanımlanmamalıdır.
void *usta_thread(void *arg) {
    int id = *(int *)arg;
    Usta usta = ustalar[id];

    // Usta başında sahip olunan ve olunmayan malzemeleri yazdır
    usta_bilgi_yazdir(id, usta.sahip1, usta.sahip2);

    while (1) {
        sem_wait(&usta_sem[id]);

        pthread_mutex_lock(&mutex);
        if (mevcut_index > toplam_satir) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        if (mevcut_index == toplam_satir) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        MalzemeCifti cift = malzeme_listesi[mevcut_index++];
        pthread_mutex_unlock(&mutex);
        printf("------------------------------------------\n");
        printf("Usta %d: %s ve %s aldı. Salata yapıyor...\n", id + 1, malzeme_adi(cift.malzeme1), malzeme_adi(cift.malzeme2));
        sleep(rand() % 5 + 1);
        printf("Usta %d: Salatayı hazırladı.\n", id + 1);
        printf("Usta %d: Salatayı toptancıya teslim etti.\n", id + 1);
        sem_post(&toptanci_sem);
    }

    return NULL;
}

bool usta_uygun_mu(Usta usta, char m1, char m2) {
    return !(usta.sahip1 == m1 || usta.sahip1 == m2 || usta.sahip2 == m1 || usta.sahip2 == m2);
}

// KURAL: Komut satırı argümanlarının ayrıştırılmasında standart kütüphane fonksiyonları kullanılabilir.
void toptanci(char *dosya_yolu) {
    FILE *fp = fopen(dosya_yolu, "r");
    if (!fp) {
        perror("Dosya açılamadı");
        exit(EXIT_FAILURE); // KURAL: Program, herhangi bir hata durumunda stderr üzerinden anlamlı ve biçimlendirilmiş bir hata mesajı vermeli ve uygun şekilde sonlandırılmalıdır.
    }

    malzeme_listesi = malloc(sizeof(MalzemeCifti) * 100);
    char satir[MAX_LINE];
    while (fgets(satir, sizeof(satir), fp)) {
        if (strlen(satir) >= 2) {
            malzeme_listesi[toplam_satir].malzeme1 = satir[0];
            malzeme_listesi[toplam_satir].malzeme2 = satir[1];
            toplam_satir++;
        }
    }
    fclose(fp);

    for (int i = 0; i < toplam_satir; ++i) {
        char m1 = malzeme_listesi[i].malzeme1;
        char m2 = malzeme_listesi[i].malzeme2;

        printf("Toptancı: %s ve %s getirdi.\n", malzeme_adi(m1), malzeme_adi(m2));

        bool gonderildi = false;
        for (int j = 0; j < NUM_USTALAR; ++j) {
            if (usta_uygun_mu(ustalar[j], m1, m2)) {
                sem_post(&usta_sem[j]);
                sem_wait(&toptanci_sem);
                printf("Toptancı: Salatayı aldı ve satmaya gitti.\n");
                printf("------------------------------------------\n");
                gonderildi = true;
                break;
            }
        }

        if (!gonderildi) {
            printf("Hiçbir usta bu malzemeleri kullanamıyor. Malzemeler boşa gitti.\n");
        }
    }

    free(malzeme_listesi); // KURAL: Her thread, kullandığı kaynakları açıkça serbest bırakmalıdır (örneğin semafor, dinamik bellek vs.).
}

int main(int argc, char *argv[]) {
    // KURAL: Eksik veya geçersiz komut satırı argümanları ile başlatılan program, kullanım bilgilerini göstererek sonlanmalıdır.
    if (argc != 3 || strcmp(argv[1], "-i") != 0) {
        fprintf(stderr, "Kullanım: %s -i dosyaYolu\n", argv[0]);
        return EXIT_FAILURE;
    }

    pthread_t ustalar_thread[NUM_USTALAR];
    int idler[NUM_USTALAR];

    sem_init(&toptanci_sem, 0, 0); // KURAL: Tüm senkronizasyon işlemleri semaforlar aracılığıyla sağlanmalıdır.
    for (int i = 0; i < NUM_USTALAR; ++i)
        sem_init(&usta_sem[i], 0, 0);

    for (int i = 0; i < NUM_USTALAR; ++i) {
        idler[i] = i;
        pthread_create(&ustalar_thread[i], NULL, usta_thread, &idler[i]);
    }

    toptanci(argv[2]);

    for (int i = 0; i < NUM_USTALAR; ++i)
        sem_post(&usta_sem[i]);

    for (int i = 0; i < NUM_USTALAR; ++i)
        pthread_join(ustalar_thread[i], NULL); // KURAL: Program kesinlikle zombi süreç üretmemelidir.

    for (int i = 0; i < NUM_USTALAR; ++i)
        sem_destroy(&usta_sem[i]); // KURAL: Her thread, kullandığı kaynakları açıkça serbest bırakmalıdır (örneğin semafor, dinamik bellek vs.).
        
    sem_destroy(&toptanci_sem); // KURAL: Her thread, kullandığı kaynakları açıkça serbest bırakmalıdır (örneğin semafor, dinamik bellek vs.).
    pthread_mutex_destroy(&mutex); // KURAL: Her thread, kullandığı kaynakları açıkça serbest bırakmalıdır (örneğin semafor, dinamik bellek vs.).aldı.
    return EXIT_SUCCESS;
}

// KURAL: Bellek kullanımı ve süreç yönetimiyle ilgili problemler (örneğin bellek sızıntısı, zombi süreçler) varsa, bu durumları analiz eden araçlarla tespit edildiği gösterilmelidir.
// (Bu madde kodda doğrudan gösterilemez, analiz araçları ile dışarıdan kontrol edilir.)
