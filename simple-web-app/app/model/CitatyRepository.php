<?php
namespace Poradna;
use Nette;

/**
 *  * Tabulka citatu
 *   */
class CitatyRepository extends Repository
{	
	/**
	 * Vybere a vrati nahodny citat (radek) z tabulky citatu
	 * @return type Jeden radek z tabulky citatu
	 */
	public function getRandom()
    {
		$tmp = $this->getTable();
		return $tmp[rand(1, $tmp->count())];
    }
}
