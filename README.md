README of the SC NEST simulation

Icíar Martínez Comba (imcomba@correo.ugr.es)

In order to execute the following files NEST Simulator installation is needed: http://www.nest-simulator.org/installation/

'MyModuleSC': A directory containing the neural modules needed for the stimulus of the neural system.
	'sc_generator':Contains the parameters needed for the generator device, as it needs to produce a certain input current.
IMPORTANT: It needs to be defined as the neuron type of a population when used in NEST.
	'scmodule':New neural module created to implement the generator device. IMPORTANT: It needs to be installed at the beginning of each NEST simulation with the command: nest.Install('scmodule')

'Scripts': A directory containing different simulations created in order to obtain certain results to study the Superior Colliculus behaviour under some specific circumstances
IMPORTANT: The simulation is developed through Jupyter notebook.
For a better understanding of the followed procedure there is an order in the creation of each simulation:
	1)'input_current'
	2)'FEF_response'
	3)'adapt_time_constant'
	4)'syn_strength'
	5)'burstproperties_lateralconections'
	6)'saccade_amplitude'
	7)'saccade_amplitud_2'
	8)'k_21_deg_sacc'
	9)'eye_movement'

All this investigation is explained and analysed at 'Memoria_TFG_SC.pdf'   
	
