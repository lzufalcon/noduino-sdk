/*
 *  Copyright (c) 2015 - 2025 MaiKe Labs
 *
 *  A tiny crontab and timeout (at) implementation for the esp8266. Resolution
 *  is in secondes. The crontab use the TIMER0 of an esp8266! Please try to work
 *  with the timeout and crontab methods to realize timer based solutions
 *
 *  Based on AnalyserDude & other reference implementations
 *  v.1.1.0	
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
*/

#include "cron.h"

char nextidentifyer;
static volatile os_timer_t cron_timer;

irom void cron_increment_clock(void){
    ++system_millisecunds_since_startup;
    ++cron_ms_for_execution_timer;
    if(system_millisecunds_since_startup >= 86400000){
        // 24h
        system_millisecunds_since_startup = 0;
        ++system_days_since_startup;
        if(system_days_since_startup == 255){
            system_days_since_startup = 0;
        }
    }
}

irom void cron_execute_jobs(void){
    // check first
    if(crontab_ptr->first == NULL || (cron_ms_for_execution_timer < crontab_ptr->first->total_ms)){
        // too low, nothing can ever match
        return;
    }
    // reset on end
    if(cron_ms_for_execution_timer > crontab_ptr->last->total_ms){
        cron_ms_for_execution_timer = 1;
    }
    // check elements
    cron_joblist_t  *current;
    current         = crontab_ptr->first;
    while(current != NULL &&  current->total_ms <= cron_ms_for_execution_timer){
        if( (cron_ms_for_execution_timer % current->total_ms) == 0 ){
            current->fn();
        }
        current = current->next;
    }
}

irom void cron_execute_timeout(void){
    cron_timeoutlist_t  *current;
    current         = timeout_ptr->first;
    while(current != NULL){
        --current->ms_left;
        if(current->ms_left <= 0){
            current->fn();
        }
        current = current->next;
    }
}

irom void cron_timerfunc(void){
    // tick the clock
    cron_increment_clock();
    // should execute a job?
    cron_execute_jobs();
    // should execute a job?
    cron_execute_timeout();
}

irom void cron_calculate_uptime_hms(){
    // ms +(sec *1000) +(min *60 *1000) +(hours *60 *60 *1000);
    
    const uint32_t s = abs(system_millisecunds_since_startup /1000);

    cron_hours      = floor( s /60 /60);
    cron_minutes    = floor( (s -(cron_hours *60 *60)) /60);
    cron_seconds    = floor( s  -((cron_hours *60 *60)
                                +(cron_minutes *60)));
}

irom void cron_init(void){
    // clear global timer
    system_millisecunds_since_startup   = 0;
    system_days_since_startup           = 0;
    cron_seconds                        = 0;
    cron_minutes                        = 0;
    cron_hours                          = 0;
    
    // set the identifyer, will be increased by every new job
    nextidentifyer = 1;
    
    // initialize the crontab
    crontab_ptr = malloc(sizeof(cron_borders_t));
    crontab_ptr->first                  = NULL;
    crontab_ptr->last                   = NULL;
    
    // initialize the timeouts
    timeout_ptr = malloc(sizeof(cron_timeoutborders_t));
    timeout_ptr->first                  = NULL;
    timeout_ptr->last                   = NULL;
    
#ifndef IGNOREINTESTS
	os_timer_disarm(&cron_timer); 
	os_timer_setfn(&cron_timer, (os_timer_func_t *) cron_timerfunc, NULL);
	os_timer_arm(&cron_timer, 1000, 1);
#endif //IGNOREINTESTS
}

irom char cron_add_job(int ms, int sec, int min, int hours, void *fn){
    // create a new job
    cron_joblist_t *newCronJob  = malloc(sizeof(cron_joblist_t));
    newCronJob->identifyer      = nextidentifyer++;
    newCronJob->fn              = fn;

    // sort by time
    uint32_t total_ms           = ms +(sec *1000) +(min *60 *1000) +(hours *60 *60 *1000);
    
    newCronJob->total_ms        = total_ms;
    newCronJob->next            = NULL;

    // move first/last and last next
    if (crontab_ptr->last       == NULL) {
        // first element
        crontab_ptr->last       = newCronJob;
        crontab_ptr->first      = newCronJob;
    } else if(crontab_ptr->last->total_ms < total_ms) {
        // add to the end
        crontab_ptr->last->next = newCronJob;
        crontab_ptr->last       = newCronJob;
    } else {
        // sort the element in the right order
        cron_joblist_t  *current;
        cron_joblist_t  *previous = NULL;
        current         = crontab_ptr->first;
        while (current != NULL){
            if(total_ms <= current->total_ms){
                if(previous == NULL){
                    crontab_ptr->first = newCronJob;
                    crontab_ptr->first->next = current;
                    break;
                } else {
                    previous->next = newCronJob;
                    newCronJob->next = current;
                    break;
                }
            }
            previous = current;
            current = current->next;
        }
    }
    return newCronJob->identifyer;
}
        
irom void cron_set_timeout(char id, int ms, int sec, int min, int hours, void *fn){
    uint32_t ms_left                = ms +(sec *1000) +(min *60 *1000) +(hours *60 *60 *1000);
    
    // if exists?
    bool changed;
    changed = false;
    cron_timeoutlist_t *current;
    current = timeout_ptr->first;
    while(current != NULL){
        if(current->identifyer == id){
            current->ms_left         = ms_left;
            current->fn              = fn;
            changed = true;
            break;
        }
        current = current->next;
    }
    if(changed == true){
        return;
    }
    cron_timeoutlist_t *newCronTimeout  = malloc(sizeof(cron_timeoutlist_t));
    newCronTimeout->identifyer      = id;
    newCronTimeout->fn              = fn;
    
    newCronTimeout->ms_left         = ms_left;
    newCronTimeout->next            = NULL;
    
    // add to list
    if (timeout_ptr->last           == NULL) {
        // first element
        timeout_ptr->last           = newCronTimeout;
        timeout_ptr->first          = newCronTimeout;
    } else {
        timeout_ptr->last->next     = newCronTimeout;
        timeout_ptr->last           = newCronTimeout;
    }
}

irom void cron_remove_job(char jobid){
    cron_joblist_t  *current;
    cron_joblist_t  *previous;
    cron_joblist_t  *next;
    previous        = NULL;
    next            = NULL;
    current         = crontab_ptr->first;
    while (current  != NULL) {
        if(current->identifyer == jobid){
            if(current == crontab_ptr->first){
                crontab_ptr->first = current->next;
            } else {
                next = current->next;
                previous->next = next;
            }
            free(current);
            break; 
        } else {
            previous    = current;
            current     = current->next;
        }
    }
}
    
irom void cron_remove_timeout(char id){
    cron_timeoutlist_t  *current;
    cron_timeoutlist_t  *previous;
    cron_timeoutlist_t  *next;
    previous        = NULL;
    next            = NULL;
    current         = timeout_ptr->first;
    while (current  != NULL) {
        if(current->identifyer == id){
            if(current == timeout_ptr->first){
                timeout_ptr->first = current->next;
            } else {
                next = current->next;
                previous->next = next;
            }
            free(current);
            break;
        } else {
            previous    = current;
            current     = current->next;
        }
    }
}

irom void cron_clear(void){
    cron_joblist_t *current;
    current = crontab_ptr->first;
    while (crontab_ptr->first != NULL) {
        current = crontab_ptr->first;
        crontab_ptr->first = current->next;
        free(current);
    }
    crontab_ptr->last = NULL;
}

irom int cron_count_jobs(void) {
    int cnt;
    cron_joblist_t *current;
    cnt = 0;
    current = crontab_ptr->first;
    while (current != NULL) {
        ++cnt;
        current = current->next;
    }
    return cnt;
}
    
irom int cron_count_timeouts(void){
    int cnt;
    cron_timeoutlist_t *current;
    cnt = 0;
    current = timeout_ptr->first;
    while (current != NULL) {
        ++cnt;
        current = current->next;
    }
    return cnt;
}
