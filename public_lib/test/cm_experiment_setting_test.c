#include <stdio.h>
#include "../cm_experiment_setting.h"

extern cm_experiment_setting_t cm_experiment_setting;
int main() {
    read_cm_experiment_setting_from_file();
    printf("interval_sec_len:%d \n \
           condition_sec_freq:%d \n \
           replacement:%d \n \
           switch_mem_type:%d \n \
           switch_drop_rate:%f \n",
           cm_experiment_setting.interval_sec_len,
           cm_experiment_setting.condition_sec_freq,
           cm_experiment_setting.replacement,
           cm_experiment_setting.switch_mem_type,
           cm_experiment_setting.switch_drop_rate);
    int i = 0;
    for (i=0; i < NUM_SWITCHES; i++) {
        printf("switch%d_interval_volume:%lu\n", i+1, cm_experiment_setting.switches_interval_volume[i]);
    }
}
