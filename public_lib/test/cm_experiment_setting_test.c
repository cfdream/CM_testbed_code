#include <stdio.h>
#include "../cm_experiment_setting.h"

extern cm_experiment_setting_t cm_experiment_setting;
int main() {
    init_cm_experiment_setting();
    printf("interval_msec_len:%d \n \
           condition_msec_freq:%d \n \
           replacement:%d \n \
           switch_mem_type:%d \n \
           switch_drop_rate:%f \n \
           uniform_mem_ratio_to_diverse_mem:%f \n \
           ",
           cm_experiment_setting.interval_msec_len,
           cm_experiment_setting.condition_msec_freq,
           cm_experiment_setting.replacement,
           cm_experiment_setting.switch_mem_type,
           cm_experiment_setting.switch_drop_rate,
           cm_experiment_setting.sample_hold_setting.uniform_mem_ratio_to_diverse_mem);
    int i = 0;
    for (i=0; i < NUM_SWITCHES; i++) {
        printf("switch%d_interval_volume:%lu\n", i+1, cm_experiment_setting.sample_hold_setting.switches_interval_volume[i]);
    }
}
