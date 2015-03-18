''' algorithm from Hong Du
    for single scattering with Mie theory
    used to study the degree of polarization
    of incoming unpolarized sunlight in the atmosphere
    of Venus (for transit of 2012)'''


import scipy.special
import math
import matplotlib.pyplot as plt

m       = complex(1.44,0)     #ratio of refractive index of particle (can be complex) to medium
m       = m.real
#alpha   = [k*0.02/180.*math.pi for k in range(0,716)]   #scattering angle
nangle  = 361
alpha   = [k/((float(nangle)-1.0)/180.)/180.*math.pi for k in range(0,nangle)]   #scattering angle
#we use a Gamma distribution for particle sizes
lambda0 = 6.173e-7           #wavelength (in meters)

#rmean   = 1.05e-6 #mean spherical particle radius (in meters)
#sigma   = 0.26e-6 #standard deviation of particle sizes

#rmean   = 3.40e-6 #mean spherical particle radius (in meters)
#sigma   = 1.109e-6 #standard deviation of particle sizes

rmean   = 0.49e-6 #mean spherical particle radius (in meters)
sigma   = 0.22e-6 #standard deviation of particle sizes

aeff    = rmean+2.0*sigma**2/rmean
veff    = sigma**2.0/aeff/rmean

'''test to compare results with Hansen and Travis (1974)'''
'''lambda0 = 5.5e-7
aeff    = 1.e-6           
veff    = 0.25
m       = complex(1.33,0)     #ratio of refractive index of particle (can be complex) to medium
m       = m.real'''

mini=-1000 #2000
maxi=-mini+1
nm=(maxi-mini)

a=[k/float(maxi)*aeff*2.0+2.01*aeff for k in range(mini,maxi,1)]
weight=[k**((1.0-3.0*veff)/veff)*math.exp(-k/aeff/veff) for k in a] #normal distribution of particle sizes
totalweight=sum(weight)

plt.clf()
plt.plot([k*1.e6 for k in a],[k/totalweight for k in weight],color="r")
plt.xlabel("radius in micrometers")
plt.ylabel("weight")
plt.title("Aerosol size distribution")
plt.savefig("distribution.png")

#linpol = [[0.0]*201 for k in range(721)] #linpol is now a 2D array: linpol[721][201]
linpol =[0.0]*nangle
F11    =[0.0]*nangle
F21    =[0.0]*nangle

for size in range(0,nm):  #loop on sizes

    print size,len(a)
    x=2.0*math.pi*a[size]/lambda0
    #cotmx = (complex(0,1)+math.tan(x*m.real)-math.exp(-2.0*x*m.imag)*math.tan(x*m.real)+complex(0,1)*math.exp(-2.0*x*m.imag))/(-1.0+complex(0,1)*math.tan(x*m.real)+complex(0,1)*math.exp(-2.0*x*m.imag)*math.tan(x*m.real)+math.exp(-2.0*x*m.imag))
    #cotmx=cotmx.real
    cotmx=1.0/math.tan(m*x)

    for k in range(0,nangle): #loop on angles
        
        rn        = []
        BesselPsi = []
        BesselEta = []
        BesselPsim= []
        BesselEtam= []
        
        rn.append(cotmx)#for degree n=0
        BesselPsi.append(math.cos(x)) #n=-1
        BesselPsi.append(math.sin(x)) #n= 0
        BesselEta.append(complex(math.cos(x),-math.sin(x))) #n=-1
        BesselEta.append(complex(math.sin(x), math.cos(x))) #n=0
        '''BesselPsim.append(math.cos(m*x)) #n=-1
        BesselPsim.append(math.sin(m*x)) #n= 0
        BesselEtam.append(complex(math.cos(m*x),-math.sin(m*x))) #n=-1
        BesselEtam.append(complex(math.sin(m*x), math.cos(m*x))) #n=0'''
        
        Pin = [0.0,1.0]
        Taun= [0.0,math.cos(alpha[k])]
        S1  = complex(0.0,0.0)
        S2  = complex(0.0,0.0)
        #Qext= 0.0
        
        for i in range(1,int(math.ceil(x*1.25))+1): #loop on the degree
            rn.append(1.0/((2.0*(float(i)-1.0)+1.0)/m/x-rn[i-1]))
            #BesselPsim.append((2.0*(float(i)-1.0)+1.0)*BesselPsim[i]/x-BesselPsim[i-1])
            #BesselEtam.append((2.0*(float(i)-1.0)+1.0)*BesselEtam[i]/x-BesselEtam[i-1])
            BesselPsi.append((2.0*(float(i)-1.0)+1.0)*BesselPsi[i]/x-BesselPsi[i-1])
            BesselEta.append((2.0*(float(i)-1.0)+1.0)*BesselEta[i]/x-BesselEta[i-1])
            an = ( ((rn[i]/m+float(i)*(1.0-1.0/m/m)/x)*BesselPsi[i+1]-BesselPsi[i])/ ((rn[i]/m+float(i)*(1.0-1.0/m/m)/x)*BesselEta[i+1]-BesselEta[i]) )
            bn = ( (rn[i]*m*BesselPsi[i+1]-BesselPsi[i])/ (rn[i]*m*BesselEta[i+1]-BesselEta[i]) )
            if i>1:
                Pin.append(Pin[i-1]*(2.0*math.cos(alpha[k])+math.cos(alpha[k])/(float(i)-1.0))-Pin[i-2]*(1.0+1.0/(float(i)-1.0)))
                Taun.append(float(i)*(math.cos(alpha[k])*Pin[i]-Pin[i-1])-Pin[i-1])
            S1 += (2.0*float(i)+1.0)/float(i)/(float(i)+1.0)*(an*Pin[i]+bn*Taun[i])
            S2 += (2.0*float(i)+1.0)/float(i)/(float(i)+1.0)*(bn*Pin[i]+an*Taun[i])
            #temp=an+bn
            #Qext += 2.0/x/x*(2.0*float(i)+1.0)*temp.real

        F11[k] += (abs(S1)**2.0+abs(S2)**2.0)*weight[size]/totalweight #S1 is at a given angle and a given size
        F21[k] += (abs(S1)**2.0-abs(S2)**2.0)*weight[size]/totalweight
        #linpol2 = 100.0*(abs(S2)-abs(S1))/(abs(S1)+abs(S2)) #degree of linear polarization for a given size and angle
        #linpol[k] += linpol2*weight[size]/totalweight #weighted average of linear polarization

#linpol = [100.0*(abs(j)-abs(k))/(abs(j)+abs(k))  for j,k in zip(S02,S01)]
linpol = [100.0*j/k  for j,k in zip(F21,F11)]

#print k,len(alpha),alpha[k]/math.pi*180.,Qext,S1,S2,linpol[k]
#linpol=[k.real for k in linpol]

alpha =[k*180./math.pi for k in alpha]

#print BesselPsi,BesselEta
plt.clf()
plt.plot(alpha,linpol,color="r")
plt.title("Single scattering with Mie formalism")
plt.xlabel("Scattering angle")
plt.ylabel("Degree of polarization in %")
plt.savefig("MieScattering.png")
