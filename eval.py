import numpy as np
import matplotlib.pyplot as plt
import json
import math 
import os
from statistics import mean

def levenshtein_distance(obtained, expected):

    size_1 = len(obtained) + 1
    size_2 = len(expected) + 1

    matrix = np.zeros((size_1, size_2))

    for i in range(size_1):
        matrix[i, 0] = i

    for j in range(size_2):
        matrix[0, j] = j

    for i in range(1, size_1):
        for j in range(1, size_2):

            if obtained[i-1] == expected[j-1]:
                matrix[i, j] = min(matrix[i-1, j-1], matrix[i-1, j] + 1, matrix[i, j - 1] + 1)
            else:
                matrix[i, j] = min(matrix[i-1, j] + 1, matrix[i-1, j-1] + 1, matrix[i, j - 1] + 1)

    return matrix[size_1 - 1, size_2 - 1]


def accuracy(obtained, expected):
    distance = levenshtein_distance(obtained, expected)
    max_size = max(len(obtained), len(expected), 1)

    return 1-(distance/max_size)

def main():
    if not os.path.exists("plotResults"):
        
        # if the demo_folder directory is not present 
        # then create it.
        os.makedirs("plotResults")

    # Opening JSON file
    fileObtained = open('obtained.json')
    fileExpected = open('expected.json')
    
    jsonObtained = json.load(fileObtained)
    jsonExpected = json.load(fileExpected)


    acc_results = dict()
    error_results = dict()
    leagueNumberImages = dict()
    keyNumberImages = dict()


    myKeys = list(jsonObtained.keys())
    myKeys = sorted(myKeys, key=str.casefold)
    jsonObtained = {i: jsonObtained[i] for i in myKeys}

    totalNumberImages = 0

    for league in jsonObtained:

        numberImages = len(jsonObtained[league])

        totalNumberImages += numberImages
        leagueNumberImages[league] = numberImages

        acc_results[league] = dict()
        error_results[league] = dict()

        for image in jsonObtained[league]:
            for key in jsonObtained[league][image]:

                # initialize once
                if key not in acc_results[league]:
                    acc_results[league][key] = dict()
                    error_results[league][key] = dict()

                # control of number of images per key
                if key not in keyNumberImages:
                    keyNumberImages[key] = 1
                else:
                    keyNumberImages[key] += 1

                for method in jsonObtained[league][image][key]:
                    
                    obtained = jsonObtained[league][image][key][method]
                    expected = jsonExpected[league][image][key]

                    acc = 0
                    error = 0

                    acc = accuracy(obtained, expected) * 100
                    if (acc != 100):
                        error = 1

                    if method in acc_results[league][key]:
                        acc_results[league][key][method] += [acc]
                        error_results[league][key][method] += error
                    else:
                        acc_results[league][key][method] = [acc]
                        error_results[league][key][method] = error


    finalResult = dict()
    errorFinalResult = dict()

    for league in acc_results:

        count_1 = count_2 = 0
        index_1 = round(math.sqrt(len(acc_results[league])))
        index_2 = math.ceil(math.sqrt(len(acc_results[league])))

        figure, axis = plt.subplots(index_1, index_2)
        
        for key in acc_results[league]:
            # initialize once
            if key not in finalResult:
                finalResult[key] = dict()
                errorFinalResult[key] = dict()

            methods = []
            accuracies = []
            errors = []

            ylabel = ""

            points = []

            for method in acc_results[league][key]:
                points.append((method, round(mean(acc_results[league][key][method]), 2), error_results[league][key][method]))

            points.sort(key = lambda x: x[1])  

            for point in points:

                method = point[0]
                acc = point[1]
                error = point[2]

                methods.append(method)
                accuracies.append(acc)
                errors.append(error)

                if (not ylabel):
                    ylabel = "[" + str(acc) + "%"
                else : 
                    ylabel += ", " + str(acc) + "%"

                if method not in finalResult[key]:
                    finalResult[key][method] = [acc]
                    errorFinalResult[key][method] = error
                else:
                    finalResult[key][method] += [acc]
                    errorFinalResult[key][method] += error

            
            ylabel += "]"

            axes = None

            if (index_1 == 1):
                axes = axis[count_2]
                plt.subplots_adjust(left=0.1,
                        right=0.93,
                        top=0.8,
                        wspace=0.4,
                        hspace=0.4)
            else:
                axes = axis[count_1, count_2]
                plt.subplots_adjust(left=0.1,
                        right=0.93,
                        wspace=0.3)
                
            colorBar = 'mediumaquamarine'
            colorText = 'black'
            if (key == 'downDistance'):
                colorText = colorBar = 'maroon'
                
            bar = axes.bar(methods, accuracies, color=colorBar, width=0.3)
            axes.bar_label(bar, errors, color=colorText)
            axes.set_ylabel(ylabel, color=colorText)
            axes.set_title(key, color=colorText)

            if (count_2 + 1 < index_2):
                count_2 += 1
            else:
                count_2 = 0
                count_1 += 1
            
            
        plt.suptitle('Accuracy testing in ' + league + '\n(' + str(leagueNumberImages[league]) + ' image(s))', 
                fontsize='large',
                fontweight='bold',
                style='italic',
                family='monospace')
        figure.set_figwidth(10)

        if (index_1 == 1):
            figure.set_figheight(5)
        else:
            figure.set_figheight(9)
        
        plt.savefig('plotResults/' + league + '.png')
        plt.show()


    # final graph
    count_1 = count_2 = 0
    index_1 = round(math.sqrt(len(finalResult)))
    index_2 = math.ceil(math.sqrt(len(finalResult)))

    figure, axis = plt.subplots(index_1, index_2)

    for key in finalResult:

        methods = []
        accuracies = []
        errors = []

        ylabel = ""

        points = []

        for method in finalResult[key]:
            points.append((method, round(mean(finalResult[key][method]), 2), errorFinalResult[key][method]))

        points.sort(key = lambda x: x[1])  

        for point in points:

            method = point[0]
            acc = point[1]
            error = point[2]

            methods.append(method)
            accuracies.append(acc)
            errors.append(error)

            if (not ylabel):
                ylabel = "[" + str(acc) + "%"
            else : 
                ylabel += ", " + str(acc) + "%"

        ylabel += "]"

        axes = None

        if (index_1 == 1):
            axes = axis[count_2]
            plt.subplots_adjust(left=0.1,
                    right=0.93,
                    top=0.8,
                    wspace=0.4,
                    hspace=0.4)
        else:
            axes = axis[count_1, count_2]
            plt.subplots_adjust(left=0.1,
                    right=0.93,
                    wspace=0.3)
        
        colorBar = 'mediumaquamarine'
        colorText = 'black'
        if (key == 'downDistance'):
            colorText = colorBar = 'maroon'

        bar = axes.bar(methods, accuracies, color=colorBar, width=0.3)
        axes.bar_label(bar, errors, color=colorText)
        axes.set_ylabel(ylabel, color=colorText)
        axes.set_title(key + '('+ str(keyNumberImages[key]) + ' image(s))', color=colorText)

        if (count_2 + 1 < index_2):
            count_2 += 1
        else:
            count_2 = 0
            count_1 += 1
        
        
    plt.suptitle('Final accuracy testing\n' + '(' + str(totalNumberImages) + ' image(s))', 
                fontsize='large',
                fontweight='bold',
                style='italic',
                family='monospace')
    figure.set_figwidth(17)

    if (index_1 == 1):
        figure.set_figheight(5)
    else:
        figure.set_figheight(9)

    plt.savefig('plotResults/final.png')
    plt.show()


if __name__ == "__main__":
    main()