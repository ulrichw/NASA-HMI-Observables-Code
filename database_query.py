''' Basic Python code to run a SQL query on the JSOC database
to retrieve the polynomial-correction coefficients
and plot their time variation.
Uses the psycopg2 module
Runs on rumble with Python3.4'''

import psycopg2, numpy as np, sys
import matplotlib.pyplot as plt

def DBparams():
    DBNAME = 'jsoc'
    SERVER = 'hmidb'
    DBUSER = 'couvidat'
    DRMSPGPORT = '5432'    
    return [DBNAME,SERVER,DBUSER,DRMSPGPORT]

if __name__ == '__main__':

    dbparams=DBparams()
    # Connect to the database (and handle exceptions)
    try:
        conn=psycopg2.connect(database=dbparams[0], user=dbparams[2], host=dbparams[1], port=dbparams[3])
    except:
        print("Unable to connect to the database",dbparams[0])
        sys.exit(1)
    
    cur = conn.cursor()
    cur.execute("SELECT t_rec,coeff0,coeff1,coeff2,coeff3 FROM hmi.coefficients WHERE coeff0 IS NOT NULL and coeff0 <> 0 and coeff1 > -0.1 and coeff1 < 0.035 and coeff3 > -3e-9 and t_rec > 1050173100.0;")
    #another useful SQL query: SELECT harpnum, count(sg_000_axis000) FROM (SELECT S.harpnum, T.sg_000_axis000 FROM hmi.sharp_cea_720s_shadow AS S, hmi.sharp_cea_720s AS T WHERE S.recnum=T.recnum GROUP BY S.harpnum, T.sg_000_axis000) AS I GROUP BY harpnum HAVING count(sg_000_axis000) > 1;

    coeff0=[]
    coeff1=[]
    coeff2=[]
    coeff3=[]
    trec  =[]

    for record in cur:
        trec.append(record[0])
        coeff0.append(record[1])
        coeff1.append(record[2])
        coeff2.append(record[3])
        coeff3.append(record[4])

    trec=np.array(trec)
    coeff0=np.array(coeff0)
    coeff1=np.array(coeff1)
    coeff2=np.array(coeff2)
    coeff3=np.array(coeff3)
    trec -= min(trec)
    trec /= 86400. #converts seconds into days

    #sorting the arrays by time
    tsort=np.argsort(trec)
    trec=trec[tsort]
    coeff0=coeff0[tsort]
    coeff1=coeff1[tsort]
    coeff2=coeff2[tsort]
    coeff3=coeff3[tsort]
    
    cur.close()
    conn.close()

    #plot the polynomial coefficients as a function of time
    f,axarr=plt.subplots(2,2)
    axarr[0,0].plot(trec,coeff0,'bo')
    axarr[0,0].set_xlabel('Days since March 2010')
    axarr[0,0].set_ylabel('COEFF0')
    
    axarr[0,1].plot(trec,coeff1,'bo')
    axarr[0,1].set_xlabel('Days since March 2010')
    axarr[0,1].set_ylabel('COEFF1')
    
    axarr[1,0].plot(trec,coeff2,'bo')
    axarr[1,0].set_xlabel('Days since March 2010')
    axarr[1,0].set_ylabel('COEFF2')

    axarr[1,1].plot(trec,coeff3,'bo')
    axarr[1,1].set_xlabel('Days since March 2010')
    axarr[1,1].set_ylabel('COEFF3')

    plt.show()

    #piece-wise linear regression of COEFF0
    #deriv=np.diff(coeff0)/np.diff(trec)
    #jumps=[x for x in range(trec.shape[0]-1) if deriv[x] < -50]
    plt.clf()
    jumps=[(10,345),(365,1256),(1270,1758),(1792,3050),(3080,3917),(3930,5130)]

    #plot COEFF0 and the trend lines
    plt.plot(trec,coeff0,'b.')
    for i in range(len(jumps)):
        res=np.polyfit([x for x in range(jumps[i][0],jumps[i][1])],coeff0[jumps[i][0]:jumps[i][1]],1)
        print('Piece-wise linear regression coefficients:',res) #to see how the slopes of the trend lines vary
        plt.plot(trec[jumps[i][0]:jumps[i][1]],np.polyval(res,[x for x in range(jumps[i][0],jumps[i][1])]),'r.')

    plt.show()
