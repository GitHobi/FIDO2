#pragma once
#include "Arduino.h"
#include <iostream>
#include <vector>

template <typename T>
class CommData
{
private:
    std::vector<T> data;
    size_t dataSize = 0;

public:
    // Konstruktor
    CommData() { data.clear(); }

    CommData(CommData &source, size_t start, size_t len)
    {
        ESP_LOGI("", "Initializing data from other data source, range %i - %i", start, start + len);
        data.clear();
        auto s = source.getData();
        for (int i = start; i < (start + len); i++)
        {
            data.push_back(s[i]);
        }
    }

    // Destruktor
    ~CommData() {}

    // Methode zum Hinzufügen eines Elements zum Vektor
    void add(const T &element)
    {

        data.push_back(element);
    }

    // Methode zum Hinzufügen eines Arrays zum Vektor
    void addArray(const T *array, size_t size)
    {

        for (size_t i = 0; i < size; ++i)
        {
            data.push_back(array[i]);
        }
    }

    void addArray(const std::vector<T> &vector)
    {
        std::copy(vector.begin(), vector.end(), std::back_inserter(data));
    }

    void blowUp(size_t targetSize)
    {
        while (data.size() < targetSize + 1)
        {
            data.push_back(0x00);
        }
    }

    void poke(size_t n, const T &value)
    {
        // Gültigkeitsbereichsprüfung
        if (n >= data.size())
        {
            throw std::out_of_range("Ungültiger Index");
        }

        // Setzen des Elements an der Position n
        data[n] = value;
    }

    const T &peek(size_t n) const
    {
        // Gültigkeitsbereichsprüfung
        if (n >= data.size())
        {
            throw std::out_of_range("Ungültiger Index");
        }

        // Zurückgeben des Elements an der Position n
        return data[n];
    }

    // Methode zum Abrufen des Vektors
    const std::vector<T> &getData() const
    {
        return data;
    }

    void getData(size_t start, size_t count, CommData<T> &sub) const
    {
        if (start + count > data.size())
        {
            std::vector<T> subdata(data.begin() + start, data.end());
            sub.addArray(subdata);
        }
        else
        {
            std::vector<T> subdata(data.begin() + start, data.begin() + start + count);
            sub.addArray(subdata);
        }

        
    }

    void moveData(size_t start, size_t count, CommData<T> &sub)
    {
        getData(start, count, sub);
        if (start + count > data.size())
        {
            data.erase(data.begin() + start, data.end());
        }
        else
        {
            data.erase(data.begin() + start, data.begin() + start + count);
        }
    }

    bool getBytes(T *buffer, size_t len)
    {
        if (buffer == nullptr)
            return false;

        memcpy(buffer, (const void *)getData().data(), len);
        return true;
    }

    // Methode zum Löschen des Vektors
    void clear()
    {
        data.clear();
        dataSize = 0;
    }

    void setDataSize(size_t newSize)
    {
        dataSize = newSize;
    }

    size_t getDataSize()
    {
        return dataSize;
    }

    bool compare(size_t start, const T *array, size_t count) const
    {
        // Gültigkeitsbereichsprüfung
        if (start + count > data.size())
        {
            throw std::out_of_range("Ungültiger Bereich");
        }

        // Vergleichen der Elemente des Datenvektors mit dem Array
        for (size_t i = 0; i < count; ++i)
        {
            if (data[start + i] != array[i])
            {
                return false;
            }
        }

        // Die Elemente sind gleich
        return true;
    }

    std::vector<T> getSubArray(size_t start, size_t count)
    {
        std::vector<T> result;
        for (size_t i = 0; i < count; ++i)
        {
            result.push_back(data[start + i]);
        }
        return result;
    }

    std::vector<T> getRandomVector(size_t count)
    {
        // Erzeugen eines Vektors mit der angegebenen Größe
        std::vector<T> randomVector(count);

        // Generieren von Zufallszahlen in den Vektor
        for (size_t i = 0; i < count; ++i)
        {
            randomVector[i] = rand() % 255;
        }

        // Zurückgeben des Vektors
        return randomVector;
    }

    void replace(size_t start, const std::vector<T> &vector)
    {
        // Gültigkeitsbereichsprüfung
        if (start + vector.size() > data.size())
        {
            throw std::out_of_range("Ungültiger Bereich");
        }

        // Kopieren der Elemente aus dem Vektor in den Datenvektor
        std::copy(vector.begin(), vector.end(), data.begin() + start);
    }

    // Methode zum Drucken des Vektorinhalts
    void
    print(const char *txt = "") const
    {
        size_t pos = 0;

        std::cout << txt << std::endl;
        int count = 0;
        for (const T &element : data)
        {
            if ( count == 0 )
            {
                Serial.printf ( "%04X: ", pos);
            }

            Serial.printf("%02X ", element);
            count++;
            if (count >= 16)
            {
                count = 0;
                Serial.printf("\n");
            }

            pos++;
        }
        std::cout << std::endl;
    }
};

struct Datensatz
{
    uint8_t id[4];
    int wert;
    uint8_t key[16];
};