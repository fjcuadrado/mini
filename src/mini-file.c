/*
 * mini-file.c
 * This file is part of mini, a library to parse INI files.
 *
 * Copyright (c) 2010-2017, Francisco Javier Cuadrado <fcocuadrado@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the Francisco Javier Cuadrado nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "mini-file.h"


/**
 * Create a new SectionData structure with the given key and the given value.
 *
 * @param key Key name to create.
 * @param value Key's value to create.
 * @return On success, a new SectionData structure is returned.
 *         On failure, NULL is returned.
 */
static SectionData *
mini_file_section_data_new(const char *key, const char *value)
{
    SectionData *data;

    /* Key and value can't be NULL */
    assert(key != NULL);
    assert(value != NULL);

    data = (SectionData *) malloc(sizeof (SectionData));
    if (data == NULL)
        return NULL;

    data->key = strdup(key);
    data->value = strdup(value);
    data->next = NULL;

    return data;
}

/**
 * Free an allocated SectionData structure.
 *
 * @param data A SectionData structure.
 */
static void
mini_file_section_data_free(SectionData *data)
{
    SectionData *p;

    /* Do nothing with NULL pointers */
    if (data == NULL)
        return;

    while (data != NULL) {
        p = data;
        data = p->next;
        p->next = NULL;

        free(p->key);
        free(p->value);
        free(p);
    }
}

/**
 * Create a new Section structure containing the given section.
 *
 * @param section Section name to create.
 * @return On success, a new Section structure is returned.
 *         On failure, NULL is returned.
 */
static Section *
mini_file_section_new(const char *section_name)
{
    Section *section;

    /* Section name can't be NULL */
    assert(section_name != NULL);

    section = (Section *) malloc(sizeof (Section));
    if (section == NULL)
        return NULL;

    section->name = strdup(section_name);
    section->data = NULL;
    section->next = NULL;

    return section;
}

/**
 * Free an allocated Section structure.
 *
 * @param section A Section structure.
 */
static void
mini_file_section_free(Section *section)
{
    Section *p;

    /* Do nothing with NULL pointers */
    if (section == NULL)
        return;

    while (section != NULL) {
        p = section;
        section = p->next;
        p->next = NULL;

        mini_file_section_data_free(p->data);
        p->data = NULL;
        free (p->name);
        free (p);
    }
}

/**
 * Search a section in a given MiniFile.
 *
 * @param mini_file A MiniFile structure generated from an INI file.
 * @param section Section name to search.
 * @return On success, a Section structure is returned.
 *         On failure, NULL is returned.
 */
static Section *
mini_file_find_section(const MiniFile *mini_file, const char *section)
{
    Section *sec = NULL;

    /* MiniFile and section can't be NULL */
    assert(mini_file != NULL);
    assert(section != NULL);

    /* Search the given section into the given mini file */
    for (sec = mini_file->section; sec != NULL; sec = sec->next) {
        if (strcmp(sec->name, section) == 0)
            break;
    }

    return sec;
}

/**
 * Search a key in a given section of a MiniFile.
 *
 * @param data Section data where the given key will be searched.
 * @param key Key name to search.
 * @return On success, a SectionData structure is returned.
 *         On failure, NULL is returned.
 */
static SectionData *
mini_file_find_key(const Section *section, const char *key)
{
    SectionData *data = NULL;

    /* Data and key can't be NULL */
    assert(section != NULL);
    assert(key != NULL);

    /* Search the given key into the data of the given section */
    for (data = section->data; data != NULL; data = data->next) {
        if (strcmp(data->key, key) == 0)
            break;
    }

    return data;
}


/**
 * Create a new MiniFile structure, this structure stores the parsed INI file.
 *
 * @param file_name INI file name.
 * @return On success, a new MiniFile structure is returned.
 *         On failure, NULL is returned.
 */
MiniFile *
mini_file_new(const char *file_name)
{
    MiniFile *mini_file;

    /* Filename can't be NULL */
    if (file_name == NULL)
        return NULL;

    mini_file = (MiniFile *) malloc(sizeof (MiniFile));
    if (mini_file == NULL)
        return NULL;

    mini_file->file_name = strdup(file_name);
    mini_file->section = NULL;

    return mini_file;
}

/**
 * Free an allocated MiniFile structure.
 *
 * @param mini_file A MiniFile structure generated from an INI file.
 */
void
mini_file_free(MiniFile *mini_file)
{
    /* Do nothing with NULL pointers */
    if (mini_file == NULL)
        return;

    mini_file_section_free(mini_file->section);
    mini_file->section = NULL;

    free(mini_file->file_name);
    mini_file->file_name = NULL;

    free(mini_file);
}

/**
 * Insert a section in a MiniFile structure.
 *
 * @param mini_file A MiniFile structure generated from an INI file.
 * @param section_name Section name to insert.
 * @return On success, MiniFile structure pointer is returned.
 *         On failure, NULL is returned.
 */
MiniFile *
mini_file_insert_section(MiniFile *mini_file, const char *section_name)
{
    Section *section;

    /* MiniFile can't be NULL */
    assert(mini_file != NULL);

    /* If the section already exists, it must not be inserted. Although it must
     * be marked as the current section for future insertions */
    section = mini_file_find_section(mini_file, section_name);
    if (section == NULL) {

        /* A new section must be created and inserted */
        section = mini_file_section_new(section_name);
        if (section == NULL)
            return NULL;

        /* Insert at first position */
        section->next = mini_file->section;
        mini_file->section = section;
    }

    /* Mark the inserted section as the current section */
    mini_file->__current_section = section;

    return mini_file;
}

/**
 * Insert a key-value pair in the last readed section from an INI file.
 *
 * @param mini_file A MiniFile structure generated from an INI file.
 * @param key Key name to insert.
 * @param value Key's value to insert.
 * @return On success, MiniFile structure pointer is returned.
 *         On failure, NULL is returned.
 */
MiniFile *
mini_file_insert_key_and_value(MiniFile *mini_file, const char *key,
                               const char *value)
{
    SectionData *data;

    /* MiniFile can't be NULL */
    assert(mini_file != NULL);

    /* There isn't a section or current section */
    if (mini_file->section == NULL || mini_file->__current_section == NULL)
        return NULL;

    /* If the key already exists, it must not be inserted */
    data = mini_file_find_key(mini_file->__current_section, key);
    if (data != NULL)
        return mini_file;

    /* A new key-value pair must be created and inserted */
    data = mini_file_section_data_new(key, value);
    if (data == NULL)
        return NULL;

    data->next = mini_file->section->data;
    mini_file->section->data = data;

    return mini_file;
}

/**
 * Get the number of sections in an INI file.
 *
 * @param mini_file A MiniFile structure generated from an INI file.
 * @return The number of sections in the given INI file.
 */
unsigned int
mini_file_get_number_of_sections(MiniFile *mini_file)
{
    unsigned int num_sections = 0;
    Section *sec;

    /* MiniFile can't be NULL */
    assert(mini_file != NULL);

    /* Count sections of the MiniFile */
    for (sec = mini_file->section; sec != NULL; sec = sec->next)
        num_sections++;

    return num_sections;
}

/**
 * Get the number of keys in a given section.
 *
 * @param mini_file A MiniFile structure generated from an INI file.
 * @param section Section name to count its keys.
 * @return The number of keys in the given section is returned.
 */
unsigned int
mini_file_get_number_of_keys(MiniFile *mini_file, const char *section)
{
    unsigned int num_keys = 0;
    Section *sec;
    SectionData *data;

    /* MiniFile can't be NULL */
    assert(mini_file != NULL);

    /* Search the given section */
    sec = mini_file_find_section(mini_file, section);
    if (sec == NULL)
        return 0;

    /* Count keys of the section */
    for (data = sec->data; data != NULL; data = data->next)
        num_keys++;

    return num_keys;
}

/**
 * Get the name of the section which is in the given position.
 *
 * @param mini_file A MiniFile structure generated from an INI file.
 * @param section_pos Section's position.
 * @return On success, section name is returned.
 *         On failure, NULL is returned.
 */
char *
mini_file_get_section(MiniFile *mini_file, unsigned int section_pos)
{
    char *section_name = NULL;
    int i;
    Section *sec;

    /* MiniFile can't be NULL */
    assert(mini_file != NULL);

    /* Search the given section's position */
    sec = mini_file->section;
    for (i = 0; (i < section_pos && sec != NULL); i++)
        sec = sec->next;

    /* There is a section in the given position */
    if (sec != NULL)
        section_name = sec->name;

    return section_name;
}

/**
 * Get the name of a key which is in the given section and in the given
 * position.
 *
 * @param mini_file A MiniFile structure generated from an INI file.
 * @param section Section name from where retrieve the key name.
 * @param key_pos Key's position.
 * @return On success, key name is returned.
 *         On failure, NULL is returned.
 */
char *
mini_file_get_key(MiniFile *mini_file, const char *section,
                  unsigned int key_pos)
{
    char *key_name = NULL;
    int i;
    Section *sec;
    SectionData *data;

    /* MiniFile and section can't be NULL */
    assert(mini_file != NULL);
    assert(section != NULL);

    sec = mini_file_find_section(mini_file, section);
    if (sec == NULL)
        return NULL;

    /* Search the given key's position */
    data = sec->data;
    for (i = 0; (i < key_pos && data != NULL); i++)
        data = data->next;

    /* There is a key in the given section and in the given position */
    if (data != NULL)
        key_name = data->key;

    return key_name;
}

/**
 * Get a value from a section's key.
 *
 * @param mini_file A MiniFile structure generated from an INI file.
 * @param section Section name to retrieve key's value
 * @param key Key name to retrieve its value.
 * @return On success, the value from the given section's key is returned.
 *         On failure, NULL is returned.
 */
char *
mini_file_get_value(MiniFile *mini_file, const char *section, const char *key)
{
    Section *sec;
    SectionData *data;

    /* MiniFile can't be NULL */
    assert(mini_file != NULL);

    /* Search the given section */
    sec = mini_file_find_section(mini_file, section);
    if (sec == NULL)
        return NULL;

    /* Search the given key */
    data = mini_file_find_key(sec, key);
    if (data == NULL)
        return NULL;

    return data->value;
}
